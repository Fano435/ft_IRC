NAME = ircserver
CFLAGS = -Wall -Wextra -Werror -std=c++98
CXX = c++
SRC = main.cpp Server.cpp Client.cpp Command.cpp errors.cpp Channel.cpp
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all