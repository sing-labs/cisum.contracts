<h1 class="contract">init</h1>

The contract account sets the super-admin without resetting accounting. Other operators are authorized through cisum.auth permissions productMgmt, mallTreasury, mallAirdrop and mallPause.

<h1 class="contract">addbank</h1>

An operator authorized for productMgmt, the super-admin or the contract account registers an audited asset contract and, for fungible tokens, its exact symbol.

<h1 class="contract">advancenft</h1>

The manager advances the NFT identifier cursor after verifying occupied identifiers.

<h1 class="contract">createnft</h1>

The manager creates an NFT and offline mint-on-purchase product with a marketplace sales quota. Transfer policies, whitelists and external issuance reservations are not inspected or changed during creation. All creation steps are atomic.

<h1 class="contract">regstock</h1>

The manager registers an existing NFT for inventory sales without changing its identifier or minting supply.

<h1 class="contract">setsale</h1>

The manager changes offline product pricing, start time and purchase limits.

<h1 class="contract">setquota</h1>

The manager adjusts the cumulative sales quota subject to issued supply or actual inventory.

<h1 class="contract">setsupply</h1>

The manager changes an issuer-owned NFT maximum supply and quota without invalidating lifetime issuance.

<h1 class="contract">setmeta</h1>

The manager updates metadata for an offline NFT issued by the mall.

<h1 class="contract">setstatus</h1>

The manager lists, unlists or permanently archives a product without erasing purchase history.

<h1 class="contract">withdrawnft</h1>

The manager transfers unreserved inventory from an offline stock product.

<h1 class="contract">allocfund</h1>

The treasurer allocates retained sales funds to the airdrop budget. No external transfer occurs.

<h1 class="contract">airdrop</h1>

The distributor transfers an authorized budget in one atomic batch, with permanent batch deduplication.

<h1 class="contract">pause</h1>

The guardian pauses or resumes purchasing and airdrop execution independently.

<h1 class="contract">notifybuy</h1>

The contract emits the successfully settled purchase details; external callers cannot authorize this action.
