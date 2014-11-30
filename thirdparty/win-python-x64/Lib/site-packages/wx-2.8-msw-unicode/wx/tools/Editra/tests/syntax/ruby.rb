# Syntax Highlighting Test File for Ruby
# Some Comments about this file
# Hello World in ruby

# Keyword statement and string
puts 'Hello world'

# Function Definitions
def hello2(name)
    puts "Hello #{name}!"
end

# Class Definition
class Greeter
    def intialize(name = "World")
        @name = name
    end
    def say_hello
        puts "Hello #{@name}!"
    end
    def say_bye
        puts "Bye #{@name}, come again."
    end
end

# Keyword and some Numbers
puts 5 ** 2
