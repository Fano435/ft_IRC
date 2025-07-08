NAME = ircserver
BONUS_NAME = ircbot
CFLAGS = -Wall -Wextra -Werror -std=c++98
CXX = c++
SRC = main.cpp Server.cpp Client.cpp Command.cpp errors.cpp Channel.cpp helper.cpp
OBJ = $(SRC:.cpp=.o)

BONUS_SRC = bonus/main.cpp bonus/Bot.cpp
BONUS_OBJ = $(BONUS_SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME)

bonus: $(BONUS_NAME)

$(BONUS_NAME): $(BONUS_OBJ)
	$(CXX) $(BONUS_OBJ) -o $(BONUS_NAME)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

bonus/%.o: bonus/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BONUS_OBJ)

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all