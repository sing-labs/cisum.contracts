#!/bin/bash

## workdir: path root of repository

## env: see client.env
contract="grab11.cisum"
contract="grab11.cisum"
tcli get account $contract

tcli system newaccount flon ${contract} FU7nPo9nxMWPAcH13koNTVfdREbPq7T8ruq2aDAYm4NLtgdT6YkE --fund-account "10.0 FLON"

