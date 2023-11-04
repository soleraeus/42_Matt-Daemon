# Matt_daemon
Matt_daemon is a daemon logging server that allows clients to connect and send messages which will be logged in `/var/log/matt_daemon/matt_daemon.log`. The daemon only accepts valid UTF8 strings and it will not log empty strings nor strings containing only withespaces. It needs to be run as root.

### Installing

To install the project, you need to first clone the project.
```git clone https://github.com/soleraeus/42Matt-Daemon.git```

You will also need [gcc >= 10](https://gcc.gnu.org), perl-devel (for openssl) and Qt6.6 (for graphical client)

### Matt_daemon execution

It starts a daemon listening on port 4242 which logs every strings sent to it. Options allow you to start a more secure server on port 4343 using a hybrid encryption scheme: RSA for key exchange and AES256GCM for encrypted communication thereafter. This secure port enforces authentication and requires you to set a username and password.

The valid options are :<br>
 `-h` : Displace the help page<br>
 `-s` : Start a secure server on port 4343 in addition to the standard server running on port 4242<br>
 `-x` : Start the secure server on port 4343 only and do not start the standard one on port 4242. This option is not compatible with -s, only one of these options should be used.<br>

### Client execution

There are two clients : `Ben_AFK` and `Ben_AFK_cli`

#### Ben_AFK

`Ben_AFK` is a Qt based graphical application, which allows you to connect to the remote server and send message in either mode.
It also allows you to check the logfile from the application if you are in secure mode.
To run it, you just need to execute it after compilation. The rest is self-explanatory.

#### Ben_AFK_cli

`Ben_AFK_cli` is a command line interface program that allows you to send messages through the terminal. 
You can request the logfile by sending `log?` if you are in secure mode.
To run it, you need to give it the IP of the target. You can also give the option `-s` to connect in secure mode.

Usage: <br>
`./Ben_AFK_cli [OPTION] IP`<br>
Valid options:<br>
  `-h`: Display the help page<br>
  `-s`: Connect to the secure Matt_daemon service running at IP.<br>

