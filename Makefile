# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tnaton <marvin@42.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/11/03 11:56:11 by tnaton            #+#    #+#              #
#    Updated: 2023/11/04 16:36:47 by bdetune          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.PHONY: all
all:
	$(MAKE) -C matt_daemon
	$(MAKE) -C ben_afk_cli
	cd ben_afk; qmake; make

.PHONY: clean
clean:
	$(MAKE) clean -C matt_daemon
	$(MAKE) clean -C ben_afk_cli
	$(MAKE) clean -C ben_afk

.PHONY: fclean
fclean:
	$(MAKE) fclean -C matt_daemon
	$(MAKE) fclean -C ben_afk_cli
	$(MAKE) distclean -C ben_afk

.PHONY: re
re: fclean all
