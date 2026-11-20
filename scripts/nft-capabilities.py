#!/usr/bin/env python3
"""Inspect NFT policy ABI without signing; emit a read-only compatibility report.
The mall no longer provides setnftcaps or manages NFT transfer permissions.
Reinspect after upgrading the NFT contract. No transaction is broadcast.
"""
import argparse
import json
from pathlib import Path
import sys
from urllib.request import Request, urlopen
from urllib.error import HTTPError, URLError


def has_legacy_policy(abi):
    actions = {a['name']: a['type'] for a in abi.get('actions', [])}
    structs = {s['name']: s for s in abi.get('structs', [])}
    aliases = {t['new_type_name']: t['type'] for t in abi.get('types', [])}

    def resolve(typ):
        seen = set()
        while typ in aliases:
            if typ in seen:
                raise ValueError('cyclic ABI type alias')
            seen.add(typ)
            typ = aliases[typ]
        return typ

    def fields(typ, seen=None):
        typ = resolve(typ)
        seen = set() if seen is None else seen
        if typ in seen or typ not in structs:
            raise ValueError('invalid ABI action structure')
        seen.add(typ)
        struct = structs[typ]
        inherited = fields(struct['base'], seen) if struct.get('base') else []
        return inherited + [resolve(field['type']) for field in struct['fields']]

    # Both calls form the existing policy initialization protocol. Do not infer
    # compatible wire layouts merely from action names.
    expected = {'setpolicy': ['uint64', 'name'], 'setxferwl': ['uint64', 'name', 'bool']}
    return all(name in actions and fields(actions[name]) == types for name, types in expected.items())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument('--abi', type=Path, help='local NFT ABI file')
    source.add_argument('--rpc', help='chain RPC endpoint; read-only get_abi')
    parser.add_argument('--nft', required=True, help='NFT contract account')
    parser.add_argument('--op-id', type=int, help='deprecated; ignored (mall setnftcaps was removed)')
    args = parser.parse_args()
    try:
        if args.abi:
            abi = json.loads(args.abi.read_text())
        else:
            request = Request(args.rpc.rstrip('/') + '/v1/chain/get_abi',
                              data=json.dumps({'account_name': args.nft}).encode(),
                              headers={'Content-Type': 'application/json'}, method='POST')
            with urlopen(request, timeout=15) as response:
                result = json.load(response)
            if result.get('account_name') != args.nft or not result.get('abi'):
                raise ValueError('RPC returned no ABI for the requested NFT account')
            abi = result['abi']
        if not isinstance(abi.get('actions'), list) or not isinstance(abi.get('structs'), list):
            raise ValueError('ABI must contain actions and structs')
        print(json.dumps({'bank': args.nft, 'has_policy': has_legacy_policy(abi),
                          'note': 'Read-only inspection; mall.cisum does not manage NFT policies'}))
    except HTTPError as error:
        parser.exit(1, f'ABI inspection failed: HTTP {error.code}\n')
    except URLError:
        parser.exit(1, 'ABI inspection failed: RPC unavailable\n')
    except (ValueError, KeyError, TypeError, OSError) as error:
        parser.exit(1, f'ABI inspection failed: {type(error).__name__}\n')

if __name__ == '__main__':
    main()
