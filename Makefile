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

NAME = Matt_daemon

BEN_AFK = Ben_AFK/Ben_AFK

BEN_AFK_CLI = Ben_AFK_cli/Ben_AFK_cli

$(NAME) : 
	$(MAKE) -C matt_daemon

$(BEN_AFK) :
	cd Ben_AFK; qmake; make

$(BEN_AFK_CLI):
	$(MAKE) -C Ben_AFK_cli

.PHONY: all
all: $(NAME) $(BEN_AFK_CLI) $(BEN_AFK)

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
