# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tnaton <marvin@42.fr>                      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/09/27 17:55:22 by tnaton            #+#    #+#              #
#    Updated: 2023/09/27 21:07:29 by bdetune          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

-include $(OBJS:%.o=%.d)

vpath %.hpp inc

NAME = Matt_daemon

OBJDIR := obj

SRCDIR := srcs

SRCS = main.cpp \
	   Tintin_reporter.cpp \
	   Server.cpp

INC = Matt_daemon.hpp \
	  Tintin_reporter.hpp \
	  Server.hpp

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

$(NAME): $(OBJS) $(INC)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

$(OBJS): $(INC)

$(OBJS) : | $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I inc -o $@ -c $<

$(OBJDIR):
	test -d $@ || mkdir -p $@

.SECONDARY: $(OBJS)

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
