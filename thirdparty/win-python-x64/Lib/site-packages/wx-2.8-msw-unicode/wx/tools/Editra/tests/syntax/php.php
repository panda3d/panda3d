<?php
    // Syntax Highlighting Test File for PHP
    /* Some comments about this file */
    // Comment line
    $hello = "HELLO"
    $world = "WORLD"

    include_once($hello_root_path . 'hellolib.php');

    function print_mood()
    {
        switch($_GET['friendly'])
        {
            case "yes":
                echo "<h1>$hello $world</h1>";
                break;
            case "no":
                echo "<h1>Bah!!</h1>"
                break;
            default:
                echo "<h1>$hello???</h1>";
        } 
    }

    /* Class Definition Test */
    class Foo
    {
        var $myvalue;

        function bar()
        {
            if (isset($this))
            {
                echo '$this is defined (';
                echo get_class($this);
                echo ")\n";
            } else {
                echo "\$this is not defined.\n";
            }
        }
        function helloA(param) {
            echo "$param";
        }
        function printEndTag() {
            echo "?>"
        }
        function printStartTag() {
            echo "<?php"
        }

    }

    function hello(param) {
        echo "$param";
    }
?>

<html>
   <head>
      <!-- Some Embedded HTML -->
      <title>Hello.php</title>
   </head>
   <body>
      <div>
        <p>Today is <?php disp_date() ?> and this website says <?php print_mood() ?></p>
      </div>
   </body>
</html>

