# Syntax Highlighting Test File for Bash
# Comment Line

# Test Assignment
BLUE="[34;01m"
CYAN="[36;01m"
GREEN="[32;01m"
RED="[31;01m"
YELLOW="[33;01m"
OFF="[0m"

echo "${YELLOW}**${OFF} Hello World in Yellow ${YELLOW}**${OFF}"
sleep 2

# Test Scalar
HELLO=$(Hello)
HELLO2=${Hello}
HELLO3=`Hello`

# Test Loop/Condition
if [ 'a' == 'a' ]; then
    for file in $( ls ); do
        echo $file
    done
fi

# Test Function Definition
function quit {
    exit
}

# Here Statement
cat <<EOF
This is a multiline block for testing the
highlighting of a here statement
EOF
