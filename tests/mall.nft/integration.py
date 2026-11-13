#!/usr/bin/env python3
"""Real-chain checks for strict creation/transfer authorization and mall compatibility."""
import importlib.util
import json
from pathlib import Path
import subprocess
import time
import sys

sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('mall_test_helpers', ROOT/'tests/mall.cisum/integration.py')
t = importlib.util.module_from_spec(spec)
spec.loader.exec_module(t)
run, cli, act, rows, nft = t.run, t.cli, t.act, t.rows, t.nft


def state():
    return [rows('mall.nft', scope, table) for scope, table in [
        ('mall.nft','tokenstats'),('mall.nft','issuance'),('manager','accounts'),
        ('alice','accounts'),('bob','accounts'),('mall.cisum','accounts')]]


def main():
    abi = json.loads((ROOT/'build/mall-verified/mall.nft.abi').read_text())
    assert {'creators','tokenstats','accounts','issuance','xferpolicy','xferwhite'} <= {x['name'] for x in abi['tables']}
    assert 'setpolicy' in {x['name'] for x in abi['actions']}
    assert not {'setnftwhite','addwhitelist','delwhitelist'} & {x['name'] for x in abi['actions']}
    # Table byte layouts and index order must match mall.cisum's external table readers.
    reference = json.loads((ROOT/'build/mall-verified/ticket.cvnft.abi').read_text())
    for table in ['tokenstats','accounts']:
        fields=[]
        for a in [reference,abi]:
            ty=next(x['type'] for x in a['tables'] if x['name']==table)
            fields.append(next(x['fields'] for x in a['structs'] if x['name']==ty))
        assert fields[0]==fields[1], table
    run('docker','run','-d','--rm','--network','none','--platform','linux/amd64',
        '--name',t.CONTAINER,'-v',f'{ROOT}/build/mall-verified:/artifacts:ro',
        '--entrypoint','bash','fucli:v0.8.3','-lc','sleep infinity')
    try:
        run('docker','exec','-d',t.CONTAINER,'bash','-lc',
            'funod --data-dir /tmp/chain --config-dir /tmp/config --enable-stale-production '
            '--producer-name flon --plugin eosio::chain_api_plugin --plugin eosio::producer_plugin '
            '--http-server-address 127.0.0.1:8888 --http-validate-host=false --wasm-runtime eos-vm '
            '--max-transaction-time 500 --abi-serializer-max-time-ms 1000 >/tmp/node.log 2>&1')
        run('docker','exec','-d',t.CONTAINER,'bash','-lc',
            'mkdir -p /root/flon-wallet && chmod 700 /root/flon-wallet && '
            'fuwal --wallet-dir /tmp/wallet --http-server-address 127.0.0.1:8900 >/tmp/wallet.log 2>&1')
        for command in [('get','info'),('wallet','list')]:
            for _ in range(60):
                try:
                    cli(*command)
                    break
                except RuntimeError:
                    time.sleep(.5)
            else:
                raise RuntimeError('local chain/wallet startup failed')
        cli('wallet','create','--to-console')
        cli('wallet','import','--private-key',t.DEV_KEY)
        for account in ['mall.nft','mall.cisum','cisum.token','manager','alice','bob']:
            cli('create','account','flon',account,t.PUB,t.PUB)
        for account in ['mall.nft','mall.cisum','cisum.token']:
            artifact = 'global.mall.nft' if account == 'mall.nft' and '--global-fixture' in sys.argv else account
            cli('set','contract',account,'/artifacts',artifact+'.wasm',artifact+'.abi','-p',account)
        if '--global-fixture' in sys.argv:
            act('mall.nft','setcreator',['manager',True],'mall.nft')
            act('mall.nft','create',['manager',10,{'nid':70},'ipfs://upgrade','manager'],'manager')
            act('mall.nft','issue',['manager',nft(2,70),'existing balance'],'manager')
            act('mall.nft','setxferwl',['manager',True],'mall.nft')
            act('mall.nft','setcreator',['manager',False],'mall.nft')
            cli('set','contract','mall.nft','/artifacts','mall.nft.wasm','mall.nft.abi','-p','mall.nft')
            assert rows('mall.nft','mall.nft','xfermode')[0]['mode']==1
            act('mall.nft','transfer',['manager','alice',[nft(1,70)],'old global permission'],'manager','sender not in NFT')
        create=['manager',10,{'nid':77},'ipfs://strict-77','manager']
        # Empty creator list does not open creation; unauthorized users cannot grant themselves access.
        act('mall.nft','create',create,'manager','creator not authorized')
        act('mall.nft','setcreator',['manager',True],'alice','authority')
        act('mall.nft','setcreator',['manager',True],'mall.nft')
        assert rows('mall.nft','mall.nft','xfermode')==[]
        act('mall.nft','create',create,'alice','authority')
        act('mall.nft','create',create,'manager')
        act('mall.nft','issue',['manager',nft(5,77),'issue'],'manager')
        # Even the issuer cannot transfer until explicitly whitelisted.
        act('mall.nft','transfer',['manager','alice',[nft(2,77)],'blocked'],'manager','sender not in NFT')
        act('mall.nft','setxferwl',[77,'manager',True],'manager')
        act('mall.nft','transfer',['manager','alice',[nft(2,77)],'allowed'],'manager')
        act('mall.nft','setxferwl',[77,'bob',True],'manager')
        before=state()
        act('mall.nft','transfer',['alice','bob',[nft(1,77)],'receiver cannot bypass'],'alice','sender not in NFT')
        cli('push','action','mall.nft','transfer',json.dumps(['alice','bob',[nft(1,77)],'cosign']),
            '-p','alice','-p','mall.nft',fail='sender not in NFT')
        assert state()==before
        act('mall.nft','setpolicy',[77,'bob'],'alice','authority')
        act('mall.nft','setpolicy',[77,'bob'],'manager')
        act('mall.nft','setpolicy',[77,'alice'],'manager','immutable')
        act('mall.nft','setxferwl',[77,'alice',True],'alice','requires issuer')
        act('mall.nft','setxferwl',[77,'alice',True],'bob')
        act('mall.nft','transfer',['alice','bob',[nft(1,77)],'allowed'],'alice')
        act('mall.nft','setxferwl',[77,'alice',False],'manager')
        act('mall.nft','transfer',['alice','bob',[nft(1,77)],'revoked'],'alice','sender not in NFT')
        # NFT 77 permissions must not authorize transfers of NFT 78.
        act('mall.nft','create',['manager',10,{'nid':78},'ipfs://strict-78','manager'],'manager')
        act('mall.nft','issue',['manager',nft(2,78),'issue'],'manager')
        act('mall.nft','setxferwl',[78,'alice',True],'bob','requires issuer')
        before=state()
        act('mall.nft','transfer',['manager','alice',[nft(1,77),nft(1,78)],'batch'],'manager','sender not in NFT')
        assert state()==before
        act('mall.nft','setxferwl',[78,'manager',True],'manager')
        act('mall.nft','transfer',['manager','alice',[nft(1,77),nft(1,78)],'batch allowed'],'manager')
        act('mall.nft','setxferwl',[77,'manager',False],'manager')
        act('mall.nft','transfer',['manager','bob',[nft(1,77)],'revoked 77'],'manager','sender not in NFT')
        act('mall.nft','transfer',['manager','bob',[nft(1,78)],'78 unaffected'],'manager')
        act('mall.nft','setxferwl',[77,'manager',True],'manager')
        act('mall.nft','transfer',['manager','alice',[],'empty'],'manager','1 to 50')
        if '--global-fixture' in sys.argv:
            # Existing assets survive upgrade; grant only their specific transfer permission.
            act('mall.nft','setpolicy',[70,'manager'],'manager')
            act('mall.nft','setxferwl',[70,'manager',True],'manager')
            act('mall.nft','transfer',['manager','alice',[nft(1,70)],'upgraded existing asset'],'manager')
        # Creator revocation denies new templates, including when the list becomes empty.
        act('mall.nft','setcreator',['manager',False],'mall.nft')
        act('mall.nft','create',['manager',10,{'nid':79},'ipfs://strict-79','manager'],'manager','creator not authorized')
        # Existing issuer rights remain valid. Burns never replenish the lifetime cap.
        act('mall.nft','retire',[nft(1,77),'burn'],'manager')
        act('mall.nft','setmaxsup',[77,5],'manager')
        act('mall.nft','issue',['manager',nft(1,77),'reissue'],'manager','lifetime issuance')
        # Mall atomic create and purchase through the new backend.
        cli('set','account','permission','mall.cisum','active','--add-code','-p','mall.cisum')
        act('mall.cisum','init',['manager']*4,'mall.cisum')
        act('mall.cisum','addbank',['mall.nft',True,'4,CISUM'],'mall.cisum')
        act('mall.cisum','addbank',['cisum.token',False,'4,CISUM'],'mall.cisum')
        data=['mall.nft','ipfs://mall-item','manager',100,50,'cisum.token',t.terms(),1,'create']
        act('mall.cisum','createnft',data,'manager','creator not authorized')
        assert rows('mall.cisum','mall.cisum','products')==[]
        assert int(rows('mall.cisum','mall.cisum','config')[0]['next_nft'])==6000000001
        act('mall.nft','setcreator',['mall.cisum',True],'mall.nft')
        act('mall.cisum','createnft',data,'manager')
        assert rows('mall.nft',6000000001,'xferwhite')==[]
        assert not any(int(r['nft_id'])==6000000001 for r in rows('mall.nft','mall.nft','xferpolicy'))
        act('mall.cisum','setstatus',[1,1,89,'listing without policy'],'manager')
        act('mall.nft','setpolicy',[6000000001,'manager'],'mall.cisum')
        act('mall.nft','setxferwl',[6000000001,'mall.cisum',True],'manager')
        assert int(rows('mall.cisum','mall.cisum','products')[0]['nft_id'])==6000000001
        act('mall.cisum','setstatus',[1,1,2,'list'],'manager')
        act('cisum.token','init',['manager']*3,'cisum.token')
        act('cisum.token','create',['manager','1000.0000 CISUM'],'cisum.token')
        act('cisum.token','issue',['manager','100.0000 CISUM','test'],'manager')
        for account in ['manager','alice','mall.cisum']:
            act('cisum.token','addwhitelist',[account],'cisum.token')
        act('cisum.token','transfer',['manager','alice','50.0000 CISUM','test'],'manager')
        act('cisum.token','transfer',['alice','mall.cisum','20.0000 CISUM','mall:v1:1:2:order1'],'alice')
        received=next(r for r in rows('mall.nft','alice','accounts') if int(r['balance']['symbol']['nid'])==6000000001)
        assert int(received['balance']['amount'])==2
        assert int(rows('mall.cisum','cisum.token','funds')[0]['sales'])==200000
        act('mall.nft','transfer',['alice','bob',[nft(1)],'no transfer permission'],'alice','sender not in NFT')
        act('mall.nft','setxferwl',[6000000001,'alice',True],'manager')
        act('mall.nft','transfer',['alice','bob',[nft(1)],'allowed'],'alice')
        act('mall.nft','transfer',['alice','bob',[nft(1,78)],'other NFT not authorized'],'alice','sender not in NFT')
        act('mall.nft','setxferwl',[6000000001,'mall.cisum',False],'manager')
        before=state()
        funds_before=rows('mall.cisum','cisum.token','funds')
        balances_before=rows('cisum.token','alice','accounts')
        act('cisum.token','transfer',['alice','mall.cisum','10.0000 CISUM','mall:v1:1:1:blocked'],'alice','sender not in NFT')
        assert state()==before
        assert rows('mall.cisum','cisum.token','funds')==funds_before
        assert rows('cisum.token','alice','accounts')==balances_before
        # Original ticket.cvnft has no setpolicy/setreserve; creation must still succeed.
        cli('create','account','flon','ticket.cvnft',t.PUB,t.PUB)
        cli('set','contract','ticket.cvnft','/artifacts','ticket.cvnft.wasm','ticket.cvnft.abi','-p','ticket.cvnft')
        act('mall.cisum','addbank',['ticket.cvnft',True,'4,CISUM'],'mall.cisum')
        act('mall.cisum','createnft',['ticket.cvnft','ipfs://plain-backend','manager',100,20,'cisum.token',t.terms(),90,'plain create'],'manager')
        plain=rows('ticket.cvnft','ticket.cvnft','tokenstats')
        assert len(plain)==1 and int(plain[0]['max_supply']['amount'])==100
        assert int(plain[0]['supply']['amount'])==0
        # Listing/quota and atomic purchases work without policy or issuance extensions.
        act('mall.cisum','setquota',[2,25,91,'plain quota'],'manager')
        act('mall.cisum','setstatus',[2,1,92,'plain listing'],'manager')
        def plain_state():
            return [rows(bank,scope,table) for bank,scope,table in [
                ('ticket.cvnft','ticket.cvnft','tokenstats'),
                ('ticket.cvnft','mall.cisum','accounts'),('ticket.cvnft','alice','accounts'),
                ('cisum.token','alice','accounts'),('cisum.token','mall.cisum','accounts'),
                ('mall.cisum','mall.cisum','products'),('mall.cisum','mall.cisum','receipts'),
                ('mall.cisum','alice','purchases'),('mall.cisum','cisum.token','funds')]]
        before=plain_state()
        act('cisum.token','transfer',['alice','mall.cisum','10.0000 CISUM','mall:v1:2:1:plain'],'alice','transfer blocked')
        assert plain_state()==before
        act('ticket.cvnft','addwhitelist',['mall.cisum'],'ticket.cvnft')
        act('cisum.token','transfer',['alice','mall.cisum','10.0000 CISUM','mall:v1:2:1:plain'],'alice')
        assert int(rows('ticket.cvnft','alice','accounts')[0]['balance']['amount'])==1
        # Existing inventory: register before deposit, receive, list and sell.
        act('ticket.cvnft','create',['manager',10,{'nid':42},'ipfs://stock','manager'],'manager')
        act('ticket.cvnft','issue',['manager',nft(3,42),'stock'],'manager')
        act('mall.cisum','regstock',['ticket.cvnft',42,0,'cisum.token',t.terms(),93,'plain stock'],'manager')
        act('ticket.cvnft','transfer',['manager','mall.cisum',[nft(3,42)],'mallstock:v1'],'manager')
        act('mall.cisum','setquota',[3,2,94,'stock quota'],'manager')
        act('mall.cisum','setstatus',[3,1,95,'stock listing'],'manager')
        act('cisum.token','transfer',['alice','mall.cisum','10.0000 CISUM','mall:v1:3:1:stock'],'alice')
        received=next(r for r in rows('ticket.cvnft','alice','accounts') if int(r['balance']['symbol']['nid'])==42)
        assert int(received['balance']['amount'])==1
        print('PASS: creator permissions, per-NFT policy/whitelist isolation, batch rollback, marketplace integration and optional global-version migration')
    finally:
        run('docker','rm','-f',t.CONTAINER)

if __name__=='__main__':
    main()
