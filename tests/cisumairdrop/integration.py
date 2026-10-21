#!/usr/bin/env python3
"""Integration checks against a fresh, prefunded TESTNET deployment."""

import argparse
import json
import subprocess
from decimal import Decimal


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", required=True)
    parser.add_argument("--url", required=True)
    parser.add_argument("--contract", default="cisumairdrop")
    parser.add_argument("--admin", required=True)
    parser.add_argument("--alice", required=True)
    parser.add_argument("--bob", required=True)
    parser.add_argument("--other-token-contract", required=True)
    parser.add_argument("--other-symbol", default="8,SING", help="precision,SYMBOL")
    args = parser.parse_args()
    other_precision, other_code = args.other_symbol.split(",")
    other_precision = int(other_precision)
    assert 0 <= other_precision <= 18 and other_code.isalpha() and other_code.isupper()
    assert (args.other_token_contract, args.other_symbol) != ("cisum.token", "4,CISUM")
    contract, admin, alice, bob = args.contract, args.admin, args.alice, args.bob
    assert len({contract, admin, alice, bob}) == 4, "use distinct test accounts"

    def cli(*command):
        return subprocess.run(
            [args.cli, "--url", args.url, *command],
            text=True, capture_output=True, check=False,
        )

    def read(*command):
        result = cli(*command)
        assert result.returncode == 0, result.stderr
        return json.loads(result.stdout)

    def balance(account, bank="cisum.token", code="CISUM"):
        result = cli("get", "currency", "balance", bank, account, code)
        assert result.returncode == 0, result.stderr
        value = result.stdout.strip()
        return Decimal(value.split()[0]) if value else Decimal(0)

    def table(name):
        return read("get", "table", contract, contract, name)["rows"]

    def snapshot():
        return (
            [balance(a, bank, code) for bank, code in
             (("cisum.token", "CISUM"), (args.other_token_contract, other_code))
             for a in (contract, alice, bob)],
            table("batches"), table("global"),
        )

    def push(action, data, signer, error=None, bank="cisum.token", sym="4,CISUM"):
        if action == "batchdrop":
            data = [data[0], bank, sym, *data[1:]]
        before = snapshot() if error else None
        result = cli("push", "action", contract, action, json.dumps(data), "-p", signer + "@active")
        if error:
            assert result.returncode != 0, f"{action} unexpectedly succeeded"
            assert error in result.stderr + result.stdout, result.stderr + result.stdout
            assert snapshot() == before, "failed action changed balances or tables"
        else:
            assert result.returncode == 0, result.stderr + result.stdout

    def item(to=alice, quantity="0.1"):
        return {"to": to, "quantity": quantity}

    def query(action, data):
        before = snapshot()
        result = read("push", "action", contract, action, json.dumps(data), "-p", alice + "@active", "--json")
        assert snapshot() == before, "query changed balances or tables"
        return result["processed"]["action_traces"][0]["return_value_data"]

    def other_amount(value):
        return f"{Decimal(value):.{other_precision}f}"

    assert balance(contract, args.other_token_contract, other_code) >= Decimal(10), "prefund secondary token with at least 10 units"
    assert table("global") == [], "requires a fresh, uninitialized contract"
    assert table("batches") == [], "requires an unused batch table"
    assert balance(contract) >= Decimal("1"), "prefund at least 1.0000 CISUM"
    assert int(query("lastbatch", [])) == 0
    push("getbatch", [1], alice, "batch not found")
    push("getbatch", [0], alice, "batch_id must be positive")
    push("batchdrop", [1, [item()], "test"], admin, "contract not initialized")
    push("init", [admin], contract)
    push("init", [admin], contract, "contract already initialized")
    push("batchdrop", [1, [item()], "test"], alice, "admin/contract only")
    push("setpause", [True], alice, "admin/contract only")
    push("setadmin", [bob], alice, "admin/contract only")
    push("setadmin", [""], admin, "admin account does not exist")

    before = [balance(a) for a in (contract, alice, bob)]
    recipients = [item(), item(bob, "0.20")]
    push("batchdrop", [1, recipients, "integration"], admin)
    after = [balance(a) for a in (contract, alice, bob)]
    assert after == [before[0] - Decimal("0.3"), before[1] + Decimal("0.1"), before[2] + Decimal("0.2")]
    batch = table("batches")[0]
    assert batch["batch_id"] == 1 and batch["recipient_count"] == 2
    assert batch["total"] == "0.3000 CISUM" and batch["executor"] == admin
    assert batch["token_contract"] == "cisum.token"
    assert query("getbatch", [1]) == batch
    assert int(query("lastbatch", [])) == 1
    push("batchdrop", [1, recipients, "replay"], admin, "batch already processed")

    cases = [
        (0, [item()], "test", "batch_id must be positive"),
        (2, [], "test", "batch size must be between 1 and 200"),
        (2, [item()] * 201, "test", "batch size must be between 1 and 200"),
        (2, [item(), item()], "test", "duplicate recipient in batch"),
        (2, [item(), item(contract)], "test", "cannot airdrop to self"),
        (2, [item(), item("")], "test", "recipient account does not exist"),
        (2, [item(), item(bob, "0")], "test", "quantity must be positive"),
        (2, [item(quantity="-1")], "test", "invalid quantity format"),
        (2, [item(quantity="1.0000 SING")], "test", "invalid quantity format"),
        (2, [item(quantity="1.00000")], "test", "quantity exceeds token precision"),
        (2, [item()], "x" * 257, "memo too long"),
        (2, [item(quantity=f"{balance(contract) + Decimal('1'):.4f}")], "test", "insufficient token balance"),
        (2, [item(quantity="461168601842738.7903"), item(bob, "0.0001")], "test", "batch total overflow"),
    ]
    for value in ("", " ", ".1", "1.", "1..0", "+1", "1e2", "NaN", "1 CISUM", "1 ", "9" * 41):
        cases.append((2, [item(quantity=value)], "format", "invalid quantity format"))
    cases.extend([
        (2, [item(quantity="0.00000")], "precision", "quantity exceeds token precision"),
        (2, [item(quantity="461168601842738.7904")], "overflow", "quantity overflow"),
        (2, [item(quantity="461168601842739")], "scaled overflow", "quantity overflow"),
    ])
    for batch_id, entries, memo, error in cases:
        push("batchdrop", [batch_id, entries, memo], admin, error)

    push("batchdrop", [2, [item()], "invalid bank"], admin,
         "token contract account does not exist", bank="")
    push("batchdrop", [2, [item(quantity="0.100")], "wrong declared precision"], admin,
         "token symbol or precision mismatch", sym="3,CISUM")
    push("setpause", [True], admin)
    push("batchdrop", [2, [item()], "paused"], admin, "contract paused")
    assert query("getbatch", [1]) == batch
    assert int(query("lastbatch", [])) == 1
    push("setpause", [False], admin)
    # Failed batch 2 remains available for retry, including with contract authorization.
    push("batchdrop", [2, [item()], "retry"], contract)
    push("setadmin", [bob], admin)
    push("batchdrop", [3, [item()], "old admin"], admin, "admin/contract only")
    push("batchdrop", [5, [item(quantity="0.0500")], "new admin"], bob)
    push("batchdrop", [3, [item(quantity="0.0500")], "out of order"], bob)
    assert int(query("lastbatch", [])) == 5
    assert int(query("getbatch", [3])["batch_id"]) == 3
    push("getbatch", [4], alice, "batch not found")
    push("setadmin", [admin], bob)
    # A second contract/symbol has independent balances but shares replay protection.
    cisum_before = [balance(a) for a in (contract, alice, bob)]
    other_before = [balance(a, args.other_token_contract, other_code) for a in (contract, alice, bob)]
    other_recipients = [item(alice, other_amount(1)), item(bob, other_amount(2))]
    push("batchdrop", [1, other_recipients, "cross-token replay"], admin,
         "batch already processed", bank=args.other_token_contract, sym=args.other_symbol)
    push("batchdrop", [6, other_recipients, "second token"], admin,
         bank=args.other_token_contract, sym=args.other_symbol)
    assert [balance(a) for a in (contract, alice, bob)] == cisum_before
    assert [balance(a, args.other_token_contract, other_code) for a in (contract, alice, bob)] == [
        other_before[0] - 3, other_before[1] + 1, other_before[2] + 2,
    ]
    other_batch = query("getbatch", [6])
    assert other_batch["token_contract"] == args.other_token_contract
    assert other_batch["total"] == f"{other_amount(3)} {other_code}"
    assert int(query("lastbatch", [])) == 6
    print("PASS: multiple tokens, balance isolation, replay protection, queries and administration")


if __name__ == "__main__":
    main()
