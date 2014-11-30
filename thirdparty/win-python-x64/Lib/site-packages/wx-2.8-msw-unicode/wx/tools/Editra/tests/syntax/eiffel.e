-- Syntax Highlighting Test File for Eiffel
-- Some Comments about this file

class
    HELLO_WORLD
create
    make
feature
    make
        do
            io.put_string ("Hello, world!")
            io.put_new_line
            io.put_string('Unclosed String
        end
end

-- Run the HelloWorld program

program hello_prog

root
	HELLO_WORLD: "make"
cluster
	"./"
end
	include "$EIFFEL_S/library/lib.pdl"
end -- hello_prog
