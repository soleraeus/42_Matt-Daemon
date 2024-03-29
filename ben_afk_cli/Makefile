# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tnaton <marvin@42.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/27 17:55:22 by tnaton            #+#    #+#              #
#    Updated: 2023/11/04 16:35:30 by bdetune          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

-include $(OBJS:%.o=%.d)

vpath %.hpp inc

NAME = ../Ben_AFK_cli

OBJDIR := obj

SRCDIR := srcs

SRCS = main.cpp \
	   Client.cpp

INC = Client.hpp

OPENSSL = ../libs/openssl-3.0.11/libcrypto.a

MOREFLAGS = -Wformat=2				\
			-Wformat-overflow=2		\
			-Wformat-truncation=2	\
			-Wstringop-overflow=4	\
			-Winit-self				\
			-ftrapv					\
			-Wdate-time				\
			-Wconversion			\
			-Wshadow

#	-Wformat=2						Check format when call to printf/scanf...
#	-Wformat-overflow=2				Check overflow of buffer with sprintf/vsprintf
#	-Wformat-truncation=2			Check output truncation with snprintf/vsnprintf
#	-Wstringop-overflow=4			Check overflow when using memcpy and strcpy (which should not happen for obvious reason)
#	-Winit-self						Check variable which initialise themself /* int i; i = i; */
#	-ftrapv							Trap signed overflow for + - *
#	-Wdate-time						Warn if __TIME__ __DATE or __TIMESTAMP__ are encoutered to prevent bit-wise-identical compilation

CXXFLAGS = -MMD -Wall -Wextra -Werror -Wpedantic -std=c++20 -O3 -g $(MOREFLAGS)

CXX = g++

OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

$(NAME): $(OPENSSL) $(OBJS) $(INC)
	$(CXX) $(CXXFLAGS) $(OBJS) ../libs/openssl-3.0.11/libcrypto.a -ldl -pthread -o $@

$(OBJS): $(INC)

$(OBJS) : | $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I inc -I ../libs/openssl-3.0.11/include -o $@ -c $<

$(OBJDIR):
	test -d $@ || mkdir -p $@

.SECONDARY: $(OBJS)

$(OPENSSL):
	test -d ../libs/openssl-3.0.11 || (cd ../libs; tar -xvf openssl-3.0.11.tar.gz)
	cd ../libs/openssl-3.0.11; ./config no-idea no-camellia no-seed no-bf no-cast no-des no-rc2 no-rc4 no-rc5 no-md2 no-md4 no-mdc2 no-dsa no-dh no-ec no-ecdsa no-ecdh no-sock no-ssl3 no-err no-engine; make depend; make build_generated libcrypto.a

.PHONY: all
all : $(NAME)

.PHONY: clean
clean:
	rm -rf $(OBJS)

.PHONY: fclean
fclean:
	rm -rf $(NAME) $(OBJS) $(OBJDIR)

.PHONY: re
re: fclean all
