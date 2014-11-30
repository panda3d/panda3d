# Syntax Highlighting Test File for Korn Shell
# Some comments about this file

export FRIENDS="Jon Kosuke Bill Sandra"
for FRIEND in ${FRIENDS}
do
  # each loop say hello
  echo "HELLO " $FRIEND
done

exit 0
