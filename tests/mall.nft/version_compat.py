#!/usr/bin/env python3
"""Reproduce legacy mall -> global NFT setpolicy failure, then verify the current mall.
Usage: python3 tests/mall.nft/version_compat.py /path/to/legacy/mall.cisum.wasm
Requires the archived global.mall.nft.wasm/.abi fixture in build/mall-verified.
Never connects to an external chain or wallet.
"""
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import time
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('helpers', ROOT/'tests/mall.cisum/integration.py')
t = importlib.util.module_from_spec(spec)
spec.loader.exec_module(t)


def main():
    legacy = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix='mall-version-') as directory:
        fixture = Path(directory)
        (fixture/'legacy.wasm').write_bytes(legacy.read_bytes())
        t.run('docker','run','-d','--rm','--network','none','--platform','linux/amd64',
              '--name',t.CONTAINER,'-v',f'{ROOT}/build/mall-verified:/artifacts:ro',
              '-v',f'{fixture}:/fixture:ro','--entrypoint','bash','fucli:v0.8.3','-lc','sleep infinity')
        try:
            t.run('docker','exec','-d',t.CONTAINER,'bash','-lc',
                'funod --data-dir /tmp/chain --config-dir /tmp/config --enable-stale-production '
                '--producer-name flon --plugin eosio::chain_api_plugin --plugin eosio::producer_plugin '
                '--http-server-address 127.0.0.1:8888 --http-validate-host=false --wasm-runtime eos-vm '
                '--max-transaction-time 500 --abi-serializer-max-time-ms 1000 >/tmp/node.log 2>&1')
            t.run('docker','exec','-d',t.CONTAINER,'bash','-lc',
                'mkdir -p /root/flon-wallet && chmod 700 /root/flon-wallet && '
                'fuwal --wallet-dir /tmp/wallet --http-server-address 127.0.0.1:8900 >/tmp/wallet.log 2>&1')
            for cmd in [('get','info'),('wallet','list')]:
                for _ in range(40):
                    try:
                        t.cli(*cmd)
                        break
                    except RuntimeError:
                        time.sleep(.5)
                else:
                    raise RuntimeError('test chain/wallet startup timeout')
            t.cli('wallet','create','--to-console')
            t.cli('wallet','import','--private-key',t.DEV_KEY)
            for account in ['nftcreator11','mallnfttest1','cisumcisum','testtoken111']:
                t.cli('create','account','flon',account,t.PUB,t.PUB)
            t.cli('set','contract','mallnfttest1','/artifacts','global.mall.nft.wasm','global.mall.nft.abi','-p','mallnfttest1')
            t.cli('set','code','nftcreator11','/fixture/legacy.wasm','-p','nftcreator11')
            t.cli('set','abi','nftcreator11','/artifacts/mall.cisum.abi','-p','nftcreator11')
            t.cli('set','account','permission','nftcreator11','active','--add-code','-p','nftcreator11')
            t.act('nftcreator11','init',['nftcreator11']*4,'nftcreator11')
            t.act('nftcreator11','addbank',['mallnfttest1',True,'4,TOKEN'],'nftcreator11')
            t.act('nftcreator11','addbank',['testtoken111',False,'4,TOKEN'],'nftcreator11')
            t.act('mallnfttest1','setcreator',['nftcreator11',True],'mallnfttest1')
            t.act('mallnfttest1','setxferwl',['nftcreator11',True],'mallnfttest1')
            data={'bank':'mallnfttest1','uri':'xxx','ipowner':'cisumcisum','maximum':1000,
                  'quota':20,'paybank':'testtoken111','terms':{'price':'10.0000 TOKEN',
                  'starts':'2026-11-10T12:00:00','per_purchase':3,'per_account':10},
                  'op_id':1002,'reason':'first item'}
            try:
                t.act('nftcreator11','createnft',data,'nftcreator11')
            except RuntimeError as error:
                message=str(error)
                assert '8000000000000000000' in message and 'setpolicy' in message, message
                print('REPRODUCED: legacy mall -> mallnfttest1::setpolicy -> 8000000000000000000')
            else:
                raise AssertionError('legacy artifact did not reproduce the reported failure')
            assert t.rows('nftcreator11','nftcreator11','products')==[]
            assert t.rows('mallnfttest1','mallnfttest1','tokenstats')==[]
            t.cli('set','code','nftcreator11','/artifacts/mall.cisum.wasm','-p','nftcreator11')
            t.act('nftcreator11','createnft',data,'nftcreator11')
            p=t.rows('nftcreator11','nftcreator11','products')[0]
            assert int(p['nft_id'])==6000000001 and int(p['quota'])==20
            print('PASS: changing only mall WASM makes the identical createnft request succeed; NFT and op_id rollback verified')
        finally:
            t.run('docker','rm','-f',t.CONTAINER)

if __name__=='__main__':
    main()
