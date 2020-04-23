CXX = g++
CXXFLAGS = -std=c++14 -Wall -Werror -g -MMD
EXEC = lang
OBJECTS = start.o tokens.o parser.o grammars.o symbol.o generate.o startup.o
DEPENDS = ${OBJECTS:.o=.d}

${EXEC}: ${OBJECTS}
	${CXX} ${CXXFLAGS} ${OBJECTS} -o ${EXEC}

-include ${DEPENDS}

clean:
	rm ${OBJECTS} ${EXEC} ${DEPENDS}
.PHONY: clean