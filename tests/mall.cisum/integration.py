#!/usr/bin/env python3
"""Run real WASM tests in a disposable, network-isolated FLON chain.
Requires Docker, fucli:v0.8.3 and artifacts from build.sh. No host ports or wallets.
"""
import json
from pathlib import Path
import subprocess
import time
import uuid
import sys
from concurrent.futures import ThreadPoolExecutor

ROOT = Path(__file__).resolve().parents[2]
CONTAINER = 'mall-test-' + uuid.uuid4().hex[:10]
# Publicly documented development genesis key, used only inside the disposable chain.
PUB = 'FU6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV'
DEV_KEY = '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3'
OP = 0
OLD_ARRAY = "--roles-fixture" in sys.argv
LEGACY = "--upgrade-fixture" in sys.argv or OLD_ARRAY


def run(*args, fail=None):
    p = subprocess.run(args, text=True, capture_output=True)
    if fail is not None:
        assert p.returncode != 0 and fail in p.stdout + p.stderr, (args, p.stdout, p.stderr)
        return
    if p.returncode:
        raise RuntimeError(f'{args[:5]} failed: {p.stdout}\n{p.stderr}')
    return p.stdout


def cli(*args, fail=None):
    return run('docker', 'exec', CONTAINER, 'fucli', '--wallet-url', 'http://127.0.0.1:8900', *args, fail=fail)


def act(bank, action, data, actor, fail=None):
    # Preserve existing scenarios while supplying the new action arguments.
    if not LEGACY and bank == 'mall.cisum' and action == 'init' and isinstance(data, list):
        data = [data[0]]
    if (not LEGACY or OLD_ARRAY) and bank == 'mall.cisum' and action == 'createnft' and isinstance(data, list) and len(data) == 9:
        data = [*data, 'business-' + str(data[7])]
    if LEGACY and OLD_ARRAY and bank == 'mall.cisum' and action == 'init':
        data = [[x] for x in data]
    business={'addbank','advancenft','createnft','regstock','setsale','setquota','setsupply','setmeta','setstatus','withdrawnft','allocfund','airdrop','pause'}
    if not LEGACY and bank == 'mall.cisum' and action in business and isinstance(data,list):
        data = [actor, *data]
    return cli('push', 'action', bank, action, json.dumps(data, ensure_ascii=False), '-p', actor, fail=fail)


def admin(action, data, fail=None, actor='manager'):
    global OP
    OP += 1
    return act('mall.cisum', action, [*data, OP, 'integration test'], actor, fail)


def rows(bank, scope, table):
    return json.loads(cli('get', 'table', bank, str(scope), table, '--limit', '1000'))['rows']


def pay(amount, memo, who='alice', fail=None):
    return act('cisum.token', 'transfer', [who, 'mall.cisum', amount, memo], who, fail)


def nft(amount, nft_id=6000000001):
    return {'amount': amount, 'symbol': {'nid': nft_id}}


def terms(start='2020-01-01T00:00:00', single=2, account=3):
    return {'price': '10.0000 CISUM', 'starts': start, 'per_purchase': single, 'per_account': account}


def snapshot():
    return {f'{b}/{s}/{t}': rows(b, s, t) for b, s, t in [
        ('mall.cisum', 'mall.cisum', 'products'), ('mall.cisum', 'cisum.token', 'funds'),
        ('mall.cisum', 'alice', 'purchases'), ('mall.nft', 'mall.nft', 'tokenstats'), ('mall.nft', 'mall.nft', 'issuance'),
        ('mall.nft', 'alice', 'accounts'), ('mall.nft', 'mall.cisum', 'accounts'),
        ('cisum.token', 'alice', 'accounts'), ('cisum.token', 'mall.cisum', 'accounts'),
        ('cisum.token', 'bob', 'accounts'), ('mall.nft', 'bob', 'accounts'),
        ('mall.cisum', 'bob', 'purchases'), ('mall.cisum', 'mall.cisum', 'batches')]}


def rejected(amount, memo, error):
    before = snapshot()
    pay(amount, memo, fail=error)
    assert snapshot() == before, 'failed transaction changed balances or accounting'


def main():
    global LEGACY
    abi=json.loads((ROOT/'build/mall-verified/mall.cisum.abi').read_text())
    actions={a['name'] for a in abi['actions']}
    assert 'receipts' not in {t['name'] for t in abi['tables']}
    assert 'init' in actions
    assert not {'setnftcaps','setxferwl','addrole','delrole'} & actions
    for name, required in [('mall.cisum', {'products', 'funds', 'purchases'}),
                           ('mall.nft', {'issuance', 'xferpolicy', 'xferwhite'})]:
        abi = json.loads((ROOT / 'build/mall-verified' / (name + '.abi')).read_text())
        assert required <= {t['name'] for t in abi['tables']}, 'missing ABI tables'
    run('docker', 'run', '-d', '--rm', '--network', 'none', '--platform', 'linux/amd64',
        '--name', CONTAINER, '-v', f'{ROOT}/build/mall-verified:/artifacts:ro',
        '--entrypoint', 'bash', 'fucli:v0.8.3', '-lc', 'sleep infinity')
    try:
        run('docker', 'exec', '-d', CONTAINER, 'bash', '-lc',
            'funod --data-dir /tmp/chain --config-dir /tmp/config --enable-stale-production '
            '--producer-name flon --plugin eosio::chain_api_plugin --plugin eosio::producer_plugin '
            '--http-server-address 127.0.0.1:8888 --http-validate-host=false --wasm-runtime eos-vm '
            '--max-transaction-time 500 --abi-serializer-max-time-ms 1000 > /tmp/node.log 2>&1')
        run('docker', 'exec', '-d', CONTAINER, 'bash', '-lc',
            'mkdir -p /root/flon-wallet && chmod 700 /root/flon-wallet && fuwal --wallet-dir /tmp/wallet --http-server-address 127.0.0.1:8900 > /tmp/wallet.log 2>&1')
        for _ in range(40):
            p = subprocess.run(['docker', 'exec', CONTAINER, 'fucli', 'get', 'info'], capture_output=True)
            if p.returncode == 0:
                break
            time.sleep(.5)
        else:
            raise RuntimeError(run('docker', 'exec', CONTAINER, 'cat', '/tmp/node.log'))
        for _ in range(40):
            p = subprocess.run(['docker', 'exec', CONTAINER, 'fucli', '--wallet-url',
                                'http://127.0.0.1:8900', 'wallet', 'list'], capture_output=True)
            if p.returncode == 0:
                break
            time.sleep(.5)
        else:
            raise RuntimeError(run('docker', 'exec', CONTAINER, 'cat', '/tmp/wallet.log'))
        cli('wallet', 'create', '--to-console')
        cli('wallet', 'import', '--private-key', DEV_KEY)
        for name in ['mall.cisum', 'mall.nft', 'cisum.token', 'manager', 'treasurer', 'distributor', 'guardian', 'alice', 'bob']:
            cli('create', 'account', 'flon', name, PUB, PUB)
        for name in ['mall.cisum', 'mall.nft', 'cisum.token']:
            artifact = ('roles.mall.cisum' if OLD_ARRAY else 'legacy.mall.cisum') if name == 'mall.cisum' and LEGACY else name
            cli('set', 'contract', name, '/artifacts', artifact+'.wasm', artifact+'.abi', '-p', name)
        act('mall.nft','setcreator',['mall.cisum',True],'mall.nft')
        act('mall.nft','setcreator',['manager',True],'mall.nft')
        cli('set', 'account', 'permission', 'mall.cisum', 'active', '--add-code', '-p', 'mall.cisum')
        act('mall.cisum', 'init', ['manager', 'treasurer', 'distributor', 'guardian'], 'mall.cisum')
        act('mall.cisum', 'addbank', ['mall.nft', True, '4,CISUM'], 'mall.cisum')
        act('mall.cisum', 'addbank', ['cisum.token', False, '4,CISUM'], 'mall.cisum')
        act('cisum.token', 'init', ['manager', 'manager', 'manager'], 'cisum.token')
        act('cisum.token', 'create', ['manager', '1000000.0000 CISUM'], 'cisum.token')
        act('cisum.token', 'issue', ['manager', '10000.0000 CISUM', 'test'], 'manager')
        for name in ['manager', 'mall.cisum', 'alice', 'bob']:
            act('cisum.token', 'addwhitelist', [name], 'cisum.token')
        for name in ['alice', 'bob']:
            act('cisum.token', 'transfer', ['manager', name, '1000.0000 CISUM', 'test'], 'manager')
        admin('createnft', ['mall.nft', 'ipfs://first', 'manager', 10, 8, 'cisum.token', terms('2099-01-01T00:00:00')])
        if LEGACY:
            before=rows('mall.cisum','mall.cisum','config')[0]
            cli('set','contract','mall.cisum','/artifacts','mall.cisum.wasm','mall.cisum.abi','-p','mall.cisum')
            LEGACY=False
            assert rows('mall.cisum','mall.cisum','config')[0]==before
        # Upgrade requires explicit super-admin initialization; legacy arrays grant nothing.
        if '--upgrade-fixture' in sys.argv or OLD_ARRAY:
            act('mall.cisum','setstatus',[1,1,100001,'before init'],'manager','initialize unified')
            if not OLD_ARRAY:
                assert not rows('mall.cisum','mall.cisum','products')[0].get('product_code')
        act('mall.cisum','init',['manager'],'mall.cisum')
        cli('create','account','flon','cisum.auth',PUB,PUB)
        cli('set','contract','cisum.auth','/artifacts','flon.auth.wasm','flon.auth.abi','-p','cisum.auth')
        act('cisum.auth','init',['manager'],'cisum.auth')
        cli('set','account','permission','cisum.auth','active','--add-code','-p','cisum.auth')
        for role,perm,user in [('treasury','mallTreasury','treasurer'),('drops','mallAirdrop','distributor'),('pause','mallPause','guardian')]:
            act('cisum.auth','addrole',['manager',role,role],'manager')
            act('cisum.auth','addroleperm',['manager',role,[perm],perm],'manager')
            act('cisum.auth','grantrole',['manager',user,role],'manager')
        assert int(rows('mall.cisum', 'mall.cisum', 'products')[0]['nft_id']) == 6000000001
        # Creation does not invoke NFT policy actions.
        assert rows('mall.nft','mall.nft','xferpolicy') == []
        admin('setstatus',[1,1])
        # Configure the created template separately; later creates also leave permissions untouched.
        act('mall.nft','setpolicy',[6000000001,'mall.cisum'],'mall.cisum')
        act('mall.nft','setxferwl',[6000000001,'mall.cisum',True],'mall.cisum')
        admin('setstatus', [1, 1])
        rejected('10.0000 CISUM', 'mall:v1:1:1:early', 'MALL_TIME')
        admin('setstatus', [1, 0]); admin('setsale', [1, terms()]); admin('setstatus', [1, 1])
        pay('20.0000 CISUM', 'mall:v1:1:2:first')
        assert int(rows('mall.nft', 'alice', 'accounts')[0]['balance']['amount']) == 2
        expected_code=rows('mall.cisum','mall.cisum','products')[0].get('product_code','')
        assert rows('mall.cisum','alice','purchases')[0]['product_code']==expected_code

        assert int(rows('mall.cisum', 'cisum.token', 'funds')[0]['sales']) == 200000
        rejected('20.0000 CISUM', 'mall:v1:1:2:first', 'MALL_LIMIT')
        rejected('30.0000 CISUM', 'mall:v1:1:3:large', 'MALL_LIMIT')
        rejected('11.0000 CISUM', 'mall:v1:1:1:odd', 'MALL_PAYMENT')
        rejected('10.0000 CISUM', 'mall:v1:1:2:wrong', 'MALL_PAYMENT')
        rejected('10.0000 CISUM', 'mall:v1:18446744073709551616:1:overflow', 'MALL_MEMO')
        act('mall.nft', 'transfer', ['alice', 'bob', [nft(1)], 'test'], 'alice', 'sender not in NFT')
        # Neither receiver whitelist nor NFT-wide legacy exemption bypasses strict policy.
        act('mall.nft', 'setxferwl', [6000000001, 'bob', True], 'mall.cisum')
        act('mall.nft', 'transfer', ['alice', 'bob', [nft(1)], 'test'], 'alice', 'sender not in NFT')
        act('mall.nft','setxferwl',[6000000001,'alice',True],'mall.cisum')
        act('mall.nft', 'transfer', ['alice', 'bob', [nft(1)], 'test'], 'alice')
        pay('10.0000 CISUM', 'mall:v1:1:1:first')  # Reused request IDs are now accepted.
        rejected('10.0000 CISUM', 'mall:v1:1:1:overcap', 'MALL_LIMIT')
        assert rows('mall.cisum','alice','purchases')[0]['product_code']==expected_code

        act('mall.nft','setxferwl',[6000000001,'mall.cisum',False],'mall.cisum')
        before = snapshot()
        pay('10.0000 CISUM', 'mall:v1:1:1:blocked', who='bob', fail='sender not in NFT')
        assert snapshot() == before
        act('mall.nft','setxferwl',[6000000001,'mall.cisum',True],'mall.cisum')
        admin('setstatus', [1, 0])
        admin('setsupply', [1, 3, 3])
        admin('setsupply', [1, 2, 3], fail='MALL_SUPPLY')
        admin('setsupply', [1, 20, 10])
        assert all(int(row['reserved'])==0 for row in rows('mall.nft','mall.nft','issuance'))
        admin('setstatus', [1, 1])
        # Force a failure in the inline NFT issue AFTER parent accounting updates.
        cli('set', 'account', 'permission', 'mall.cisum', 'active', '--remove-code', '-p', 'mall.cisum')
        before = snapshot()
        pay('10.0000 CISUM', 'mall:v1:1:1:inlinefail', who='bob', fail='authority')
        assert snapshot() == before
        act('mall.nft','setcreator',['mall.cisum',True],'mall.nft')
        act('mall.nft','setcreator',['manager',True],'mall.nft')
        cli('set', 'account', 'permission', 'mall.cisum', 'active', '--add-code', '-p', 'mall.cisum')
        # Existing NFT retains id 42 and does not require mall issuance authority.
        act('mall.nft', 'create', ['manager', 10, {'nid':42}, 'ipfs://stock', 'manager'], 'manager')
        act('mall.nft', 'issue', ['manager', nft(5,42), 'stock'], 'manager')
        act('mall.nft', 'setpolicy', [42, 'manager'], 'manager')
        for account in ['manager', 'mall.cisum']:
            act('mall.nft', 'setxferwl', [42, account, True], 'manager')
        admin('regstock', ['mall.nft', 42, 0, 'cisum.token', terms()])
        act('mall.nft', 'transfer', ['manager','mall.cisum',[nft(5,42)],'mallstock:v1'], 'manager')
        admin('setquota', [2,5]); admin('setstatus',[2,1])
        stock_before = rows('mall.nft','mall.nft','issuance')
        pay('20.0000 CISUM','mall:v1:2:2:stock')
        assert rows('mall.nft','mall.nft','issuance') == stock_before
        admin('regstock',['mall.nft',42,0,'cisum.token',terms()],fail='already registered')
        admin('setstatus',[2,0]);admin('setquota',[2,2]);admin('withdrawnft',[2,'manager',3])
        # Sales are unavailable for airdrops until explicitly allocated.
        act('mall.cisum','airdrop',[1,'cisum.token',[{'to':'bob','quantity':'1.0000 CISUM'}]],'distributor','insufficient airdrop budget')
        admin('allocfund',['cisum.token','2.0000 CISUM'],actor='treasurer')
        act('mall.cisum','airdrop',[1,'cisum.token',[{'to':'bob','quantity':'1.0000 CISUM'}]],'distributor')
        act('mall.cisum','airdrop',[1,'cisum.token',[{'to':'bob','quantity':'1.0000 CISUM'}]],'distributor','duplicate')
        pay('1.0000 CISUM','mallfund:v1')
        # Failed creation rolls back cursor and product insertion.
        cursor = rows('mall.cisum','mall.cisum','config')
        admin('createnft',['mall.nft','ipfs://first','manager',10,5,'cisum.token',terms()],fail='already exists')
        assert rows('mall.cisum','mall.cisum','config') == cursor
        policies_before=rows('mall.nft','mall.nft','xferpolicy')
        admin('createnft',['mall.nft','ipfs://second','manager',10,5,'cisum.token',terms()])
        # Even has_policy=true must never trigger permission changes during creation.
        assert rows('mall.nft','mall.nft','xferpolicy')==policies_before
        assert rows('mall.nft',6000000002,'xferwhite')==[]
        assert int(rows('mall.cisum','mall.cisum','products')[-1]['nft_id']) == 6000000002
        # Reusing an operation id cannot apply a second update.
        act('mall.cisum','setquota',[3,4,OP,'duplicate operation'],'manager','duplicate operation')
        admin('setquota',[3,4],actor='alice',fail='does not have required permission')
        admin('pause',[True,False],actor='guardian')
        rejected('10.0000 CISUM','mall:v1:1:1:paused','MALL_PAUSED')
        admin('pause',[False,False],actor='guardian')
        # A failed downstream transfer rolls back BOTH airdrop budget and batch id.
        cli('set','account','permission','mall.cisum','active','--remove-code','-p','mall.cisum')
        before=snapshot()
        act('mall.cisum','airdrop',[2,'cisum.token',[{'to':'bob','quantity':'1.0000 CISUM'}]],'distributor','authority')
        assert snapshot()==before
        cli('set','account','permission','mall.cisum','active','--add-code','-p','mall.cisum')
        act('mall.cisum','airdrop',[2,'cisum.token',[{'to':'bob','quantity':'1.0000 CISUM'}]],'distributor')
        # Removing a sender takes effect immediately, even for the original issuer.
        act('mall.nft','setxferwl',[6000000001,'alice',False],'mall.cisum')
        act('mall.nft','transfer',['alice','bob',[nft(1)],'removed'],'alice','sender not in NFT')
        # Burn does not restore lifetime issuance capacity.
        act('mall.nft','retire',[nft(1,42),'burn inventory'],'manager')
        act('mall.nft','setmaxsup',[42,4],'manager','below minted')
        act('mall.nft','setmaxsup',[42,5],'manager')
        act('mall.nft','issue',['manager',nft(1,42),'reissue'],'manager','lifetime issuance')
        # Spoofed symbol from a different token contract cannot buy or credit funds.
        cli('create','account','flon','fake.token',PUB,PUB)
        cli('set','contract','fake.token','/artifacts','cisum.token.wasm','cisum.token.abi','-p','fake.token')
        act('fake.token','init',['manager','manager','manager'],'fake.token')
        act('fake.token','create',['manager','100.0000 CISUM'],'fake.token')
        act('fake.token','issue',['manager','100.0000 CISUM','test'],'manager')
        act('fake.token','addwhitelist',['manager'],'fake.token')
        before=snapshot()
        act('fake.token','transfer',['manager','mall.cisum','10.0000 CISUM','mall:v1:1:1:fake'],'manager')
        assert snapshot()==before
        # Identifier collision skips occupied ID and does not replace existing NFT.
        act('mall.nft','create',['manager',1,{'nid':6000000003},'ipfs://occupied','manager'],'manager')
        admin('createnft',['mall.nft','ipfs://skip','manager',10,5,'cisum.token',terms()])
        assert int(rows('mall.cisum','mall.cisum','products')[-1]['nft_id'])==6000000004
        # Batch NFT transfer cannot partially move the allowed first asset.
        act('mall.nft','setxferwl',[42,'alice',True],'manager')
        before=snapshot()
        act('mall.nft','transfer',['alice','bob',[nft(1,42),nft(1)],'batch rollback'],'alice','sender not in NFT')
        assert snapshot()==before
        # Competing transactions for the last unit: exactly one settles.
        act('mall.nft','setpolicy',[6000000004,'mall.cisum'],'mall.cisum')
        act('mall.nft','setxferwl',[6000000004,'mall.cisum',True],'mall.cisum')
        admin('setquota',[4,1]);admin('setstatus',[4,1])
        sales_before=int(rows('mall.cisum','cisum.token','funds')[0]['sales'])
        def compete(who):
            try:
                pay('10.0000 CISUM',f'mall:v1:4:1:race-{who}',who=who)
                return True
            except RuntimeError as exc:
                assert 'MALL_QUOTA' in str(exc), str(exc)
                return False
        with ThreadPoolExecutor(max_workers=2) as pool:
            results=list(pool.map(compete,['alice','bob']))
        assert sum(results)==1
        assert int(rows('mall.cisum','mall.cisum','products')[-1]['sold'])==1
        assert int(rows('mall.cisum','cisum.token','funds')[0]['sales'])==sales_before+100000
        # Creation and listing do not require or modify NFT policy tables.
        policies_before=rows('mall.nft','mall.nft','xferpolicy')
        admin('createnft',['mall.nft','ipfs://no-policy','manager',10,5,'cisum.token',terms()])
        assert rows('mall.nft','mall.nft','xferpolicy')==policies_before
        admin('setstatus',[5,1])
        # Unified role checks, no local whitelist fallback, and revocation.
        act('cisum.auth','addrole',['manager','merchants','mall managers'],'manager')
        act('cisum.auth','addroleperm',['manager','merchants',['productMgmt'],'manage goods'],'manager')
        act('cisum.auth','grantrole',['manager','alice','merchants'],'manager')
        admin('setstatus',[5,0],actor='alice')
        act('mall.cisum','addbank',['cisum.token',False,'4,EXTRA'],'alice')
        assert any(r['sym']=='4,EXTRA' for r in rows('mall.cisum','cisum.token','funds'))
        before_banks=rows('mall.cisum','mall.cisum','banks')
        act('mall.cisum','addbank',['manager',True,'4,CISUM'],'bob','does not have required permission')
        assert rows('mall.cisum','mall.cisum','banks')==before_banks
        cli('push','action','mall.cisum','addbank',json.dumps(['alice','manager',True,'4,CISUM']),'-p','bob',fail='authority')

        admin('allocfund',['cisum.token','0.0001 CISUM'],actor='alice',fail='does not have required permission')
        act('cisum.auth','addallowlist',['bob'],'manager')
        admin('setstatus',[5,0],actor='bob',fail='does not have required permission')
        # A signed transaction cannot impersonate another authorized submitter.
        cli('push','action','mall.cisum','setstatus',json.dumps(['alice',5,0,109001,'spoof']),'-p','bob',fail='authority')
        act('cisum.auth','grantrole',['manager','bob','merchants'],'manager')
        admin('setstatus',[5,0],actor='bob')
        act('cisum.auth','revokerole',['manager','alice','merchants'],'manager')
        before_funds=rows('mall.cisum','cisum.token','funds')
        act('mall.cisum','addbank',['cisum.token',False,'4,REVOKED'],'alice','does not have required permission')
        assert rows('mall.cisum','cisum.token','funds')==before_funds

        before=snapshot()
        admin('setstatus',[5,1],actor='alice',fail='does not have required permission')
        assert snapshot()==before
        # External permissions may be removed without changing the mall.
        act('cisum.auth','delroleperm',['manager','merchants',['productMgmt']],'manager')
        admin('setstatus',[5,1],actor='bob',fail='does not have required permission')
        admin('pause',[False,False],actor='manager')  # super-admin bypass
        admin('setstatus',[5,0],actor='mall.cisum')  # contract bypass
        assert next(r for r in rows('mall.cisum','mall.cisum','operations') if int(r['id'])==OP)['actor']=='mall.cisum'
        # Atomic rollback even though checkrole is an inline action.
        act('cisum.auth','revokerole',['manager','distributor','drops'],'manager')
        before=snapshot()
        act('mall.cisum','airdrop',[109002,'cisum.token',[{'to':'bob','quantity':'0.0001 CISUM'}]],'distributor','does not have required permission')
        assert snapshot()==before
        act('mall.cisum','init',['alice'],'manager','authority')
        act('mall.cisum','init',['missingacct'],'mall.cisum','admin does not exist')
        act('mall.cisum','init',['alice'],'mall.cisum')
        admin('setstatus',[5,0],actor='manager',fail='does not have required permission')
        admin('setstatus',[5,0],actor='alice')
        print('PASS: purchase, rollback, limits, issuance, stock, airdrops, business IDs, unified roles, revocation and optional legacy upgrade')
    finally:
        run('docker', 'rm', '-f', CONTAINER)

if __name__ == '__main__':
    main()
