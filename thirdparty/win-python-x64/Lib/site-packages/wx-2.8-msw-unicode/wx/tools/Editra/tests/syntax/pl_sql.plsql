/**
 * Syntax Highlighting Test File for PLSQL
 * This is a package example for syntax colourising of
 * Oracle PL/SQL 11g
 * It is developed to meet at least my requirements.
 * Copyright 2007 Thomas G. Keul
 * @author Thomas G. Keul (tgkeul@web.de)
 */

 /**
  * Example pldoc header of a procedure.
  *
  * @param param_1  in parameter
  * @param param_2  out parameter
  * @param param_3  in out parameter
  * @throws no_data_found
  */
 procedure my_proc
   (param_1  in my_table.the_key%type,
    param_2 out my_table%rowtype,
    param_3  in out nocopy plsql_table_type);

 /**
  * Example pldoc header of a function.
  *
  * @param   param_1  in parameter
  * @return  true or false
  */
 function my_func (param_1 in pls_integer) return boolean;
end example
/

create or replace package body example is

 type t_boolean_table is table of boolean index by binary_integer;
 subtype t_sample is positive range 1..6;
 real_sample constant t_sample := 3;

 cursor get_it (param_1 in varchar2) is
  select /*+ a Hint */
         distinct foo
    from bar
   where the_key = param_1
     and a_column is not null;

 procedure nested_outer (param_1 in number) is
   procedure nested_inner is
   begin
     if a_collection.count > 0
     then
       for l in a_collection.first .. a_collection.last
       loop
         doit;                 -- just for fun
       end loop;
     elsif its_a_rainy_day
     then help (who => me);
     else sigh;
     end if;
   end nested_inner;

 begin
   update a_table
      set a_column = 1
    where the_key = 4711
   returning something bulk collect into a_collection;

   << a_label >>
   for ex in 1..3
   loop
     case ex
       when 1 then dbms_output.put_line ('number one');
       when 2 then junk := greatest (ex, nvl(foo, bar));
       when 3 then exit (a_label);
       else rollback;
     end case;
   end loop a_label;

 exception
   when no_data_found then null;
   when others then raise;
 end nested_outer;

begin
 oops := 'no string eol';
end example;
/
-- -----------------------------------------------------------
declare
 d date := sysdate;
begin
 dbms_output.put_line (to_char (sysdate, 'DD.MM.YYYY'));
end;
-- -----------------------------------------------------------
exec dbms_job.run(4711);
