#!/usr/bin/env bash
set -u

COUNT="${COUNT:-1000}"
RUSH_SALE_ID="${RUSH_SALE_ID:-137}"
FROM_ACCOUNT="${FROM_ACCOUNT:-myadmin}"
USERS_CSV="${USERS_CSV:-$FROM_ACCOUNT}"
TO_ACCOUNT="${TO_ACCOUNT:-grab.cisum}"
TOKEN_CONTRACT="${TOKEN_CONTRACT:-cisum.token}"
AMOUNT="${AMOUNT:-1.0000 CISUM}"
QUERY_DELAY="${QUERY_DELAY:-0.2}"
QUERY_LIMIT="${QUERY_LIMIT:-5000}"

ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
SCRIPT_DIR="$ROOT_DIR/tests/grab.cisum"
PY_HELPER="$SCRIPT_DIR/find_order_row.py"
LOG_DIR="${LOG_DIR:-$SCRIPT_DIR/logs}"
RUN_TAG="${RUN_TAG:-grab_orders_$(date +%Y%m%d_%H%M%S)}"
RUN_DIR="$LOG_DIR/$RUN_TAG"
DETAIL_CSV="$RUN_DIR/detail.csv"
SUMMARY_TXT="$RUN_DIR/summary.txt"
COMMANDS_TXT="$RUN_DIR/commands.txt"

mkdir -p "$RUN_DIR"

IFS=',' read -r -a USERS <<< "$USERS_CSV"
if [ "${#USERS[@]}" -eq 0 ]; then
  echo "USERS_CSV is empty"
  exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
  echo "python3 is required"
  exit 1
fi

submit_ok=0
submit_fail=0
query_found=0
query_missing=0
query_error=0
win_count=0
lose_count=0
switch_count=0

echo "run_tag=$RUN_TAG"
echo "logs=$RUN_DIR"
echo "count=$COUNT rush_sale_id=$RUSH_SALE_ID amount=$AMOUNT users=$USERS_CSV"

printf "idx,user,grab_id,submit_status,query_status,win_status,tickets_amount,order_id,error\n" > "$DETAIL_CSV"
: > "$COMMANDS_TXT"

run_shell_cmd() {
  local cmd="$1"
  /bin/bash -lc 'shopt -s expand_aliases; [ -f ~/.bashrc ] && source ~/.bashrc; [ -f ~/.bash_profile ] && source ~/.bash_profile; [ -f ~/.profile ] && source ~/.profile; eval "$1"' bash "$cmd"
}

make_grab_id() {
  local idx="$1"
  local user="$2"
  local seed="${RUN_TAG}:${RUSH_SALE_ID}:${user}:${idx}"
  python3 -c "import hashlib,sys; print(hashlib.md5(sys.argv[1].encode()).hexdigest())" "$seed"
}

user_idx=0
i=1
while [ "$i" -le "$COUNT" ]; do
  submit_status="ok"
  query_status="missing"
  win_status="unknown"
  tickets_amount=""
  order_id=""
  error_msg=""
  retry_count=0
  current_user=""
  grab_id=""

  while :; do
    current_user="${USERS[$user_idx]}"
    grab_id="$(make_grab_id "$i" "$current_user")"
    memo="grab:${RUSH_SALE_ID}:${grab_id}"
    payload="[\"${current_user}\",\"${TO_ACCOUNT}\",\"${AMOUNT}\",\"${memo}\"]"
    cmd="mpush ${TOKEN_CONTRACT} transfer '$payload' -p ${current_user}"

    submit_log="$RUN_DIR/submit_${i}.log"
    query_log="$RUN_DIR/query_${i}.json"
    query_err="$RUN_DIR/query_${i}.err"

    echo "$cmd" >> "$COMMANDS_TXT"

    if run_shell_cmd "$cmd" >"$submit_log" 2>&1; then
      submit_ok=$((submit_ok + 1))
      break
    fi

    error_msg="$(tr "\n" " " < "$submit_log" | sed "s/[[:space:]]\\+/ /g; s/^ //; s/ \$//")"

    case "$error_msg" in
      *"user already won a ticket in this rush sale"*|*"exceed max grabs per user"*)
        switch_count=$((switch_count + 1))
        retry_count=$((retry_count + 1))
        user_idx=$(((user_idx + 1) % ${#USERS[@]}))
        if [ "$retry_count" -lt "${#USERS[@]}" ]; then
          continue
        fi
        ;;
    esac

    submit_status="fail"
    submit_fail=$((submit_fail + 1))
    break
  done

  if [ "$submit_status" = "ok" ]; then
    sleep "$QUERY_DELAY"
    if run_shell_cmd "mcli get table \"$TO_ACCOUNT\" \"$RUSH_SALE_ID\" orders --limit \"$QUERY_LIMIT\" --json" >"$query_log" 2>"$query_err"; then
      query_result="$(python3 "$PY_HELPER" "$query_log" "$grab_id" 2>/dev/null)"
      query_rc=$?
      if [ "$query_rc" -eq 0 ]; then
        query_status="found"
        query_found=$((query_found + 1))
        order_id="${query_result%%,*}"
        tickets_amount="${query_result##*,}"
        if [ "$tickets_amount" = "1" ]; then
          win_status="win"
          win_count=$((win_count + 1))
        else
          win_status="lose"
          lose_count=$((lose_count + 1))
        fi
      else
        query_status="missing"
        query_missing=$((query_missing + 1))
        error_msg="order_not_found_by_grab_id"
      fi
    else
      query_status="error"
      query_error=$((query_error + 1))
      error_msg="$(tr "\n" " " < "$query_err" | sed "s/[[:space:]]\\+/ /g; s/^ //; s/ \$//")"
    fi
  fi

  printf "%s,%s,%s,%s,%s,%s,%s,%s,\"%s\"\n" "$i" "$current_user" "$grab_id" "$submit_status" "$query_status" "$win_status" "$tickets_amount" "$order_id" "$error_msg" >> "$DETAIL_CSV"
  echo "[$i/$COUNT] user=$current_user grab_id=$grab_id submit=$submit_status query=$query_status win=$win_status tickets=$tickets_amount"
  i=$((i + 1))
done

{
  echo "run_tag=$RUN_TAG"
  echo "commands_txt=$COMMANDS_TXT"
  echo "detail_csv=$DETAIL_CSV"
  echo "submit_ok=$submit_ok"
  echo "submit_fail=$submit_fail"
  echo "query_found=$query_found"
  echo "query_missing=$query_missing"
  echo "query_error=$query_error"
  echo "win_count=$win_count"
  echo "lose_count=$lose_count"
  echo "switch_count=$switch_count"
} | tee "$SUMMARY_TXT"
