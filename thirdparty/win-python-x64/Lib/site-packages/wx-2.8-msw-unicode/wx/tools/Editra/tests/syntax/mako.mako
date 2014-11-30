# Syntax Highlighting Test File for Mako Templates
# Comments are like this

<%def name="label(field, content)">
<label for="${field}">${content}</label>
</%def>

<%def name="make_option(opt)">  
 <%
     import os  
     from mako.template import Template  
       
     f = open("%s/templates/pages/admin/options/%s" \  
           % (c.root, opt)).read()  
     return Template(f).render()  
 %>  
</%def> 
