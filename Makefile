# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tnaton <marvin@42.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/11/03 11:56:11 by tnaton            #+#    #+#              #
#    Updated: 2023/11/03 15:22:30 by tnaton           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.PHONY: all
all:
	$(MAKE) -C matt_daemon
	$(MAKE) -C Ben_AFK_cli
	cd Ben_AFK; qmake; make

.PHONY: clean
clean:
	$(MAKE) clean -C matt_daemon
	$(MAKE) clean -C Ben_AFK_cli
	$(MAKE) clean -C Ben_AFK

.PHONY: fclean
fclean:
	$(MAKE) fclean -C matt_daemon
	$(MAKE) fclean -C Ben_AFK_cli
	$(MAKE) distclean -C Ben_AFK

.PHONY: re
re: fclean all
