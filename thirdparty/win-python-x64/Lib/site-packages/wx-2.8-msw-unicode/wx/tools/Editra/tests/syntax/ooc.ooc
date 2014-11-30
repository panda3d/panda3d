// Syntax Highlighting Sample File for OOC
// Comments are like this
import structs/ArrayList

SPACE := const ' '

StringUtilities: class {
    split: static func (string: String) -> ArrayList<String> {
        tokens := ArrayList<String> new()

        lastIndex := 0
        for(index in 0..string length()) {
            if(string[index] == SPACE) {
                token := string[lastIndex..index]

                if(!token empty?()) {
                    tokens add(token)
                }
                lastIndex = index + 1
            }
        }

        rest := string[lastIndex..string length()]
        if(!rest empty?()) {
            tokens add(rest)
        }

        return tokens
    }
}

main: func(arguments: ArrayList<String>) {
    for(argument in arguments) {
        for(token in StringUtilities split(argument)) {
            token println()
        }
    }
}
