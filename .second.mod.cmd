savedcmd_second.mod := printf '%s\n'   second.o | awk '!x[$$0]++ { print("./"$$0) }' > second.mod
