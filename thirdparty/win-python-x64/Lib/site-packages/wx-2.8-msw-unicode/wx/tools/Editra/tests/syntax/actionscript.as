// ActionScript Syntax Highlighting Test File
// Comments are like this
/** Multi-line comments are like these lines here
 *  @summary Documentation keywords are like this
 */

// Hello World ActionScript 2.0
class com.example.Greeter extends MovieClip
{
    public function Greeter()
    {
        var txtHello:TextField = this.createTextField("txtHello", 0, 0, 0, 100, 100);
        txtHello.text = "Hello World";
    }
}

// Hellow World ActionScript 3.0
package com.example
{
    import flash.text.TextField;
    import flash.display.Sprite;

    public class Greeter extends Sprite
    {
        public function Greeter()
        {
            var txtHello:TextField = new TextField();
            txtHello.text = "Hello World";
            addChild(txtHello);
        }
    }
}

// Some Variable Definitions
var item1:String="ABC";
var item2:Boolean=true;
var item3:Number=12
var item4:Array=["a","b","c"];
