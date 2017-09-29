from ubuntu:14.04

RUN apt-get update -y
RUN apt-get install gcc -y

# The version of OCaml used to compile Unison need to be identical for the server
# and client compilations. As such, pin the OCaml compiler.
# If this version of OCaml no longer exists, take care to chagne the version of the
# client as well!
RUN apt-get install ocaml=4.01.0-3ubuntu3 -y

RUN apt-get install wget -y
RUN apt-get install make -y
RUN wget http://www.seas.upenn.edu/~bcpierce/unison/download/releases/stable/unison-2.48.3.tar.gz
RUN tar zxvf unison-2.48.3.tar.gz 
RUN cd unison-2.48.3 && make UISTYLE=text
