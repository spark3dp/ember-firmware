user=root
host='192.168.7.2'
max_attempts=120 # 1 second per attempt

Red='\e[0;31m'
RCol='\e[0m'

# Wait until communication with a host on the network is possible
wait_for_connectivity() {
  echo 'Waiting for board to be ready...'
  echo

  # Attempt ssh connectivity some number of times
  ((count = $max_attempts))
  
  while [[ $count -ne 0 ]]; do
    ssh -o ConnectTimeout=1 "${user}@${host}" exit > /dev/null 2>&1
    rc=$?
    if [[ $rc -eq 0 ]]; then
      ((count = 1))
    fi
    ((count = count - 1))
  done

  if [[ $rc -ne 0 ]]; then
    printf "${Red}ERROR: Unable to reach main board, make sure USB is connected and try again, aborting${RCol}\n"
    exit 1
  fi
}

# Issue specified command(s) over ssh
send_command() {
  # Verify that a command is specified
  command="${@}"
  if [[ -z "${command}" ]]; then
    echo 'Command must be specified'
    exit 1
  fi
 
  # -t: Allow interactive session 
  # -q: Suppress output
  # -o UserKnownHostsFile=/dev/null: don't store connected hosts in the known hosts file
  # -o StrictHostKeyChecking=no: ignore about host key mismatches
  ssh -t -q -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null "${user}@${host}" "${command}"
}
