# -- Text content for the landing page.  You should change these for yours! --

title = "Landing Page"
contactInfo = "M. Ian Graham - ian.graham@dig.com - 818-623-3219"




# -- Begin fancy layout stuff, change at your own risk --

header = '''
<html><head>
<title>%(titlestring)s</title>
<style>
  body
  {
  margin: 0;
  padding: 0;
  font-size: 90%%;
  font-family: Verdana, sans-serif;
  background-color: #fff;
  color: #333;
  }

  p
  {
  margin: 0;
  padding: 10px;
  background-color: #eee;
  }

  h2
  {
  font-size: 140%%;
  color: #666;
  background-color: #fff;
  width: 22em;
  margin-left: 150px;
  margin-top: 0px;
  }

  h3
  {
  padding-top: 10px;
  margin-top: 0px;
  margin-left: 20px;
  }

  pre
  {
  margin-left: 20px;
  margin-bottom: 0px;
  padding-bottom: 10px;
  }
  
  a
  {
  text-decoration: none;
  color: #333;
  }
  
  a:hover
  {
  text-decoration: underline;
  }

  #header
  {
  margin: 0;
  padding: 0;
  background-color: #fff;
  }

  #footer
  {
  margin: 0;
  padding: 3px;
  text-align: right;
  background-color: #fff;
  border-top: 1px solid #778;
  font: 10px Verdana, sans-serif;
  }

  #contents
  {
  margin: 0;
  padding: 25;
  background-color: #eee;
  min-height:600px;
  height:auto !important;
  height:600px;
  }

  <!-- Tab menu -->

  #navcontainer
  {
  margin:0;
  padding: 0;
  }

  #navlist
  {
  padding: 3px 0;
  margin-left: 0;
  margin: 0;
  border-bottom: 1px solid #778;
  font: bold 12px Verdana, sans-serif;
  background-color: transparent;
  }

  #navlist li
  {
  list-style: none;
  margin: 0;
  display: inline;
  }

  #navlist li a
  {
  padding: 3px 0.5em;
  margin-left: 3px;
  border: 1px solid #778;
  border-bottom: none;
  background: #DDE;
  text-decoration: none;
  }

  #navlist li a:link { color: #448; }
  #navlist li a:visited { color: #667; }

  #navlist li a:hover
  {
  color: #000;
  background: #AAE;
  border-color: #227;
  }

  #navlist li a#current
  {
  background: #eee;
  border-bottom: 1px solid #eee;
  }

  #navlist li.first
  {
  margin-left: 150px;
  }

  <!-- Table formatting -->

  table
  {
  border-spacing:1px;
  background:#E7E7E7;
  color:#333;
  }
  
  caption
  {
  border: #666666;
  border-bottom: 2px solid #666666;
  margin-left: 2px;
  margin-right: 2px;
  padding: 10px;
  background: #cfcfdf;
  font: 15px 'Verdana', Arial, Helvetica, sans-serif;
  font-weight: bold;
  }
  
  td, th
  {
  font:13px 'Courier New',monospace;
  padding: 4px;
  }
  
  thead th
  {
  text-align: center;
  background: #dde;
  color: #666666;
  border: 1px solid #ffffff;
  text-transform: uppercase;
  }
  
  tbody th
  {
  font-weight: bold;
  }

  tbody tr
  {
  background: #efeffc;
  text-align: left;
  }
  
  tbody tr.odd
  {
  background: #ffffff;
  border-top: 1px solid #ffffff;
  }
  
  tbody th a:hover
  {
  color: #009900;
  }
  
  tbody tr td
  {
  text-align: left
  height: 30px;
  background: #ffffff;
  border: 1px solid #ffffff;
  color: #333;
  }
  
  tbody tr.odd td
  {
  background: #efeffc;
  border-top: 1px solid #ffffff;
  }
  
  tbody tr.dead td
  {
  background:#ff0000;
  border-top: 1px solid #ffffff;
  }

  table td a:link, table td a:visited
  {
  display: block;
  padding: 0px;
  margin: 0px;
  width: 100%%;
  text-decoration: none;
  color: #333;
  }
  
  html>body #navcontainer li a { width: auto; }
  
  table td a:hover
  {
  color: #000000;
  background: #aae;
  }
  
  tfoot th, tfoot td
  {
  background: #dfdfdf;
  padding: 3px;
  text-align: center;
  font: 14px 'Verdana', Arial, Helvetica, sans-serif;
  font-weight: bold;
  border-bottom: 1px solid #cccccc;
  border-top: 1px solid #DFDFDF;
  }  
</style>
</head>
<body>
<div id="header">
<h2>%(titlestring)s</h2>
<div id="navcontainer">
<ul id="navlist">
%(menustring)s
</ul>
</div>
</div>
<div id="contents">
<center>
'''

mainMenu = '''
<p>Whee!</p>
'''

footer = '''
</center>
</div>
<div id="footer">
Contact: %(contact)s
</div>
</body>
</html>
\r\n'''


def getRowClassString(rowNum):
    if rowNum % 2 == 0:
        return ""
    else:
        return " class=\"odd\""
