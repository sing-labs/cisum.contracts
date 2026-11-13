<h1 class="contract">setcreator</h1>

The contract account explicitly adds or removes an NFT creator. An empty creator list denies all creation.

<h1 class="contract">create</h1>

An authorized creator creates an NFT template with lifetime issuance limits. No sender is automatically whitelisted.

<h1 class="contract">issue</h1>

The issuer mints to its own account within the lifetime issuance cap.

<h1 class="contract">retire</h1>

The issuer burns its own balance without restoring lifetime issuance capacity.

<h1 class="contract">setxferwl</h1>

The issuer or designated manager adds or removes a sender for one NFT. Membership never grants permission for another NFT.

<h1 class="contract">setmaxsup</h1>

The issuer adjusts the lifetime cap without violating minted supply and reserved issuance.

<h1 class="contract">setreserve</h1>

The issuer reserves future issuance for marketplace sales.

<h1 class="contract">settokenuri</h1>

The issuer or contract account updates unique metadata.

<h1 class="contract">transfer</h1>

The sender authorizes transfer and must be whitelisted for every NFT. Recipient eligibility or contract cosigning never bypasses this requirement.

<h1 class="contract">setpolicy</h1>

The issuer assigns a whitelist manager once for an NFT. Sender whitelist enforcement is always active, including before this action.
