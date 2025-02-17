# Compiler und Flags
CXX := c++
CXXFLAGS := -Wall -Werror -Wextra -std=c++98

# Verzeichnisse
SRCDIR := server
OBJDIR := obj
CONFIG := confic

# Programmname
TARGET := webserv

# Alle .cpp-Dateien rekursiv finden
SOURCES := $(shell find $(SRCDIR) -type f -name "*.cpp")

# Ersetze Pfade für .o-Dateien (gleiche Struktur wie SOURCES, aber unter obj/)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

# Standard-Ziel
all: $(TARGET)

# Linken des finalen Programms
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Regel zum Kompilieren aller .cpp-Dateien zu .o-Dateien
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)   # 🛠 Erstelle das Verzeichnis für die .o-Datei
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Aufräumen der .o-Dateien und des obj-Verzeichnisses
clean:
	rm -rf $(OBJDIR)

# Komplettes Löschen inklusive Binary
fclean: clean
	rm -f $(TARGET)

# Kompilieren und mit der Konfigurationsdatei ausführen
confic: all
	./$(TARGET) $(CONFIG)

# Erneutes Kompilieren
re: fclean all

.PHONY: all clean fclean confic re
