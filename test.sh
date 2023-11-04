#!/bin/bash

check_ret() {
	if [ $? -eq 0 ]
	then
		if [[ $(test -e /var/lock/matt_daemon.lock; echo $?) == 1 ]]
		then
			echo -e "\e[32mSUCCESS\e[0m"
		else
			echo -e "\e[31mFAIL\e[0m"
		fi
	else
		echo -e "\e[31mFAIL\e[0m"
	fi
}

(./Matt_daemon && (echo "test"; echo "quit") | ./Ben_AFK_cli 127.0.0.1) 1>/dev/null
check_ret

cat /var/log/matt_daemon/matt_daemon.log | tail -1 | grep "Quitting" 1>/dev/null
check_ret

(echo -e "tnaton\nlemotdepasse\n" | ./Matt_daemon -x && (echo -e "tnaton\nlemotdepasse\n"; echo "salut"; echo "quit") | ./Ben_AFK_cli -s 127.0.0.1 && cat /var/log/matt_daemon/matt_daemon.log | tail -5 | grep "salut") 1>/dev/null
check_ret

echo "Expect FAIL"

(mkdir /var/lock/matt_daemon.lock && ./Matt_daemon) 2>/dev/null
check_ret
rmdir /var/lock/matt_daemon.lock

(./Matt_daemon && ( (python3 <<EOF
lst = (list(chr(i) for i in range(32, 0x110000) if chr(i).isprintable()))
print(len(lst))
while len(lst):
    str = ''.join(lst[i] for i in range(0, min(len(lst), 100)))
    print(str)
    del lst[0:100]
EOF
); echo "quit") | ./Ben_AFK_cli 127.0.0.1) 1>/dev/null
check_ret


echo "Expect FAIL"

(./Matt_daemon && ( echo -e "\x01" | ./Ben_AFK_cli 127.0.0.1); echo "quit" | ./Ben_AFK_cli 127.0.0.1; cat /var/log/matt_daemon/matt_daemon.log | tail -5 | grep 'LOG') 1>/dev/null
check_ret
