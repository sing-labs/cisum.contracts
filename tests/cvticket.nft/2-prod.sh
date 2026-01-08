#!/bin/bash
shopt -s expand_aliases
source ~/.bashrc

ticket_ntoken=ticket.cvnft
mreg flon $ticket_ntoken flonian
mtran flonian $ticket_ntoken "100 FLON"
mset $ticket_ntoken ticket.cvnft
mcli set account permission $ticket_ntoken active --add-code


mpush $ticket_ntoken  addwhitelist '["ticket.cvnft"]'   -p $ticket_ntoken


mpush $ticket_ntoken  addwhitelist '["grab.cisum"]'   -p $ticket_ntoken



mpush ticket.cvnft  settokenuri '[002101025919476,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreihqfvgozm2fyxk4kioip6m5g6rupvgtos2cwdf3e24ags4zft4f4q"]'   -p ticket.cvnft

mpush ticket.cvnft  settokenuri '[002101025898095,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreiafrvryf2znsscnd6bqmccdemqdwx24af3ddwx3ynpp57hqkb2nma"]'   -p ticket.cvnft

mpush ticket.cvnft  settokenuri '[002101025532587,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreicrlnl6376fhy7434kuhftnw6lcpc5j6yjcp7fnfvrziq4w4tbu3y"]'   -p ticket.cvnft

mpush ticket.cvnft  settokenuri '[002101025366260,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreidlxexid26hsydgeghvmsurgxejq2mlzph7iqvsm77agvfheizcii"]'   -p ticket.cvnft

mpush ticket.cvnft  settokenuri '[002101025305660,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreifrosptu2fg66njqmg3ggjq7px3nnj7avisnfvci27fq2rehrxlg4"]'   -p ticket.cvnft

mpush ticket.cvnft  settokenuri '[002101025154613,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreiaz4lq4qicjqf3lnp6mtpnbrv4apg2pvvi2ipdtvbv4qzrtnnz5oq"]'   -p ticket.cvnft

mpush ticket.cvnft  settokenuri '[002101025100721,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreibcpqfc3l4tcajv3selvtcck3cgnpilebghhad4seh63etcyvfhjq"]'   -p ticket.cvnft

mpush ticket.cvnft  settokenuri '[002101025074572,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreigpzakwfip7j6smypm5p3r5bh2q2nibuj26fo2cufioc3qqsfcj6u"]'   -p ticket.cvnft

mpush ticket.cvnft  settokenuri '[002101025210526,"https://maroon-worried-fly-573.mypinata.cloud/ipfs/bafkreid54gfebrzq5y3c2k5xjchaavy3z56rg4zo2nphcjqakdiqh7p46y"]'   -p ticket.cvnft


mpush ticket.cvnft  transfer '["grab.cisum","show.cisum",[{"amount":12,"symbol":{"nid":2101025560214}}],"initial badge"]' -p grab.cisum

mpush ticket.cvnft  transfer '["grab.cisum","show.cisum",[{"amount":12,"symbol":{"nid":2101025933580}}],"initial badge"]' -p grab.cisum

mpush ticket.cvnft  transfer '["grab.cisum","show.cisum",[{"amount":12,"symbol":{"nid":2101025260182}}],"initial badge"]' -p grab.cisum

mpush ticket.cvnft  transfer '["grab.cisum","show.cisum",[{"amount":12,"symbol":{"nid":2101025969114}}],"initial badge"]' -p grab.cisum

mpush ticket.cvnft  transfer '["grab.cisum","show.cisum",[{"amount":12,"symbol":{"nid":2101025452032}}],"initial badge"]' -p grab.cisum

mpush ticket.cvnft  retire '[{"amount":12,"symbol":{"nid":2101025560214}},"retire"]'  -p show.cisum

mpush ticket.cvnft  retire '[{"amount":12,"symbol":{"nid":2101025933580}},"retire"]'  -p show.cisum

mpush ticket.cvnft  retire '[{"amount":12,"symbol":{"nid":2101025260182}},"retire"]'  -p show.cisum

mpush ticket.cvnft  retire '[{"amount":12,"symbol":{"nid":2101025969114}},"retire"]'  -p show.cisum

mpush ticket.cvnft  retire '[{"amount":12,"symbol":{"nid":2101025452032}},"retire"]'  -p show.cisum

retire( const nasset& quantity, const string& memo )