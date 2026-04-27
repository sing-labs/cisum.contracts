#!/usr/bin/env python3
import csv
import hashlib
import json
import os
import subprocess
import sys
import threading
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from datetime import datetime


def env(name: str, default: str) -> str:
    return os.environ.get(name, default)


COUNT = int(env("COUNT", "1000"))
CONCURRENCY = int(env("CONCURRENCY", "10"))
RUSH_SALE_ID = env("RUSH_SALE_ID", "136")
FROM_ACCOUNT = env("FROM_ACCOUNT", "myadmin")
USERS_CSV = env("USERS_CSV", FROM_ACCOUNT)
TO_ACCOUNT = env("TO_ACCOUNT", "grab.cisum")
TOKEN_CONTRACT = env("TOKEN_CONTRACT", "cisum.token")
AMOUNT = env("AMOUNT", "1.0000 CISUM")
QUERY_DELAY = float(env("QUERY_DELAY", "0.2"))
QUERY_LIMIT = env("QUERY_LIMIT", "5000")

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SCRIPT_DIR = os.path.join(ROOT_DIR, "tests", "grab.cisum")
LOG_DIR = env("LOG_DIR", os.path.join(SCRIPT_DIR, "logs"))
RUN_TAG = env("RUN_TAG", f"grab_orders_{datetime.now().strftime('%Y%m%d_%H%M%S')}")
RUN_DIR = os.path.join(LOG_DIR, RUN_TAG)
DETAIL_CSV = os.path.join(RUN_DIR, "detail.csv")
SUMMARY_TXT = os.path.join(RUN_DIR, "summary.txt")
COMMANDS_TXT = os.path.join(RUN_DIR, "commands.txt")

USERS = [u.strip() for u in USERS_CSV.split(",") if u.strip()]


@dataclass
class Counters:
    submit_ok: int = 0
    submit_fail: int = 0
    query_found: int = 0
    query_missing: int = 0
    query_error: int = 0
    win_count: int = 0
    lose_count: int = 0
    switch_count: int = 0


counter_lock = threading.Lock()
file_lock = threading.Lock()
user_lock = threading.Lock()
current_user_idx = 0


def run_shell_cmd(cmd: str):
    shell_cmd = (
        'shopt -s expand_aliases; '
        '[ -f ~/.bashrc ] && source ~/.bashrc; '
        '[ -f ~/.bash_profile ] && source ~/.bash_profile; '
        '[ -f ~/.profile ] && source ~/.profile; '
        'eval "$1"'
    )
    return subprocess.run(
        ["/bin/bash", "-lc", shell_cmd, "bash", cmd],
        text=True,
        capture_output=True,
    )


def make_grab_id(idx: int, user: str) -> str:
    seed = f"{RUN_TAG}:{RUSH_SALE_ID}:{user}:{idx}"
    return hashlib.md5(seed.encode()).hexdigest()


def make_grab_key(grab_id: str) -> str:
    return hashlib.sha256(grab_id.encode()).hexdigest()


def find_order_row(output_path: str, grab_id: str):
    with open(output_path, "r", encoding="utf-8") as f:
        raw = f.read()
    start = raw.find("{")
    end = raw.rfind("}")
    if start == -1 or end == -1 or end < start:
        return None
    data = json.loads(raw[start:end + 1])
    for row in data.get("rows", []):
        if row.get("grab_id") == grab_id:
            tickets = row.get("tickets") or {}
            return str(row.get("id", "")), str(tickets.get("amount", ""))
    return None


def run_query_orders(grab_id: str):
    grab_key = make_grab_key(grab_id)
    cmd = (
        f'mcli get table "{TO_ACCOUNT}" "{RUSH_SALE_ID}" orders '
        f'--index 2 --key-type sha256 '
        f'--lower "{grab_key}" --upper "{grab_key}" --limit 1'
    )
    result = run_shell_cmd(cmd)
    return cmd, result


def next_user() -> str:
    global current_user_idx
    with user_lock:
        user = USERS[current_user_idx]
        current_user_idx = (current_user_idx + 1) % len(USERS)
        return user


def append_command(cmd: str):
    with file_lock:
        with open(COMMANDS_TXT, "a", encoding="utf-8") as f:
            f.write(cmd + "\n")


def append_detail(row):
    with file_lock:
        with open(DETAIL_CSV, "a", newline="", encoding="utf-8") as f:
            writer = csv.writer(f)
            writer.writerow(row)


def update_counters(**kwargs):
    with counter_lock:
        for key, value in kwargs.items():
            setattr(counters, key, getattr(counters, key) + value)


def normalize_error(stdout: str, stderr: str) -> str:
    return " ".join(((stdout or "") + " " + (stderr or "")).replace("\n", " ").split())


def process_one(i: int):
    submit_status = "ok"
    query_status = "missing"
    win_status = "unknown"
    tickets_amount = ""
    order_id = ""
    error_msg = ""
    retry_count = 0
    current_user = ""
    grab_id = ""

    while True:
        current_user = next_user()
        grab_id = make_grab_id(i, current_user)
        memo = f"grab:{RUSH_SALE_ID}:{grab_id}"
        payload = json.dumps([current_user, TO_ACCOUNT, AMOUNT, memo], ensure_ascii=False, separators=(",", ":"))
        cmd = f"mpush {TOKEN_CONTRACT} transfer '{payload}' -p {current_user}"

        submit_log = os.path.join(RUN_DIR, f"submit_{i}.log")
        query_log = os.path.join(RUN_DIR, f"query_{i}.json")
        query_err = os.path.join(RUN_DIR, f"query_{i}.err")

        append_command(cmd)

        result = run_shell_cmd(cmd)
        with open(submit_log, "w", encoding="utf-8") as f:
            f.write((result.stdout or "") + (result.stderr or ""))

        if result.returncode == 0:
            update_counters(submit_ok=1)
            break

        error_msg = normalize_error(result.stdout, result.stderr)
        if (
            "user already won a ticket in this rush sale" in error_msg
            or "exceed max grabs per user" in error_msg
        ):
            update_counters(switch_count=1)
            retry_count += 1
            if retry_count < len(USERS):
                continue

        submit_status = "fail"
        update_counters(submit_fail=1)
        append_detail([i, current_user, grab_id, submit_status, query_status, win_status, tickets_amount, order_id, error_msg])
        return f"[{i}/{COUNT}] user={current_user} grab_id={grab_id} submit={submit_status} query={query_status} win={win_status} tickets={tickets_amount}"

    time.sleep(QUERY_DELAY)
    query_cmd, result = run_query_orders(grab_id)
    with open(query_log, "w", encoding="utf-8") as f:
        f.write(result.stdout or "")
    with open(query_err, "w", encoding="utf-8") as f:
        f.write(f"cmd={query_cmd}\n")
        f.write(result.stderr or "")

    if result.returncode == 0:
        row = find_order_row(query_log, grab_id)
        if row is not None:
            query_status = "found"
            order_id, tickets_amount = row
            update_counters(query_found=1)
            if tickets_amount == "1":
                win_status = "win"
                update_counters(win_count=1)
            else:
                win_status = "lose"
                update_counters(lose_count=1)
        else:
            query_status = "missing"
            error_msg = "order_not_found_by_grab_id"
            update_counters(query_missing=1)
    else:
        query_status = "error"
        error_msg = normalize_error(result.stdout, result.stderr)
        update_counters(query_error=1)

    append_detail([i, current_user, grab_id, submit_status, query_status, win_status, tickets_amount, order_id, error_msg])
    message = f"[{i}/{COUNT}] user={current_user} grab_id={grab_id} submit={submit_status} query={query_status} win={win_status} tickets={tickets_amount}"
    if win_status == "win":
        message += f"\nWINNER user={current_user} grab_id={grab_id} order_id={order_id} tickets=1"
    return message


def main() -> int:
    global counters

    if not USERS:
        print("USERS_CSV is empty")
        return 1

    os.makedirs(RUN_DIR, exist_ok=True)
    counters = Counters()

    print(f"run_tag={RUN_TAG}")
    print(f"logs={RUN_DIR}")
    print(f"count={COUNT} rush_sale_id={RUSH_SALE_ID} amount={AMOUNT} users={USERS_CSV} concurrency={CONCURRENCY}")

    with open(DETAIL_CSV, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(
            ["idx", "user", "grab_id", "submit_status", "query_status", "win_status", "tickets_amount", "order_id", "error"]
        )

    with open(COMMANDS_TXT, "w", encoding="utf-8"):
        pass

    with ThreadPoolExecutor(max_workers=CONCURRENCY) as executor:
        futures = [executor.submit(process_one, i) for i in range(1, COUNT + 1)]
        for future in as_completed(futures):
            print(future.result())

    summary_lines = [
        f"run_tag={RUN_TAG}",
        f"commands_txt={COMMANDS_TXT}",
        f"detail_csv={DETAIL_CSV}",
        f"submit_ok={counters.submit_ok}",
        f"submit_fail={counters.submit_fail}",
        f"query_found={counters.query_found}",
        f"query_missing={counters.query_missing}",
        f"query_error={counters.query_error}",
        f"win_count={counters.win_count}",
        f"lose_count={counters.lose_count}",
        f"switch_count={counters.switch_count}",
    ]
    summary = "\n".join(summary_lines)
    print(summary)
    with open(SUMMARY_TXT, "w", encoding="utf-8") as f:
        f.write(summary + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
