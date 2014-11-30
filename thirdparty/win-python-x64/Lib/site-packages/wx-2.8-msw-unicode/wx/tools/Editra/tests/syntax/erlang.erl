%% Syntax Highlighting Test file for Erlang
%% Some comments about this file

%% quicksort:qsort(List)
%% Sort a list of items
-module(quicksort).
-export([qsort/1]).

qsort([]) -> [];
qsort([Pivot|Rest]) ->
    qsort([ X || X <- Rest, X < Pivot]) ++ [Pivot] ++ qsort([ Y || Y <- Rest, Y >= Pivot]).

%% --------------------------------------------------------------------- %%
%% Sort a list by length
-module(listsort).
-export([by_length/1]).

by_length(Lists) ->
    F = fun(A,B) when is_list(A), is_list(B) ->
            length(A) < length(B)
        end,
    qsort(Lists, F).

 qsort([], _)-> [];
 qsort([Pivot|Rest], Smaller) ->
     qsort([ X || X <- Rest, Smaller(X,Pivot)], Smaller)
     ++ [Pivot] ++
     qsort([ Y ||Y- Rest, not(Smaller(Y, Pivot))], Smaller).

