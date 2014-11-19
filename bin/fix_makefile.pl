#!/usr/bin/env perl

use strict;
use warnings;

# usage => perl bin/fix_makefile.pl target_nginx_dir/objs/Makefile

sub main {
    my $file_path =$ARGV[0];
    my @arr = ();

    open my $in, "<", $file_path;
    while (my $line = <$in>) {
        push @arr, $line;
    }
    close $in;

    my $lines = join "", @arr;

    $lines =~ s|CFLAGS =  -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -|CFLAGS =  -pipe  -O3 -W -Wall -Wpointer-arith -Wno-unused-parameter -Werror -|;
    $lines =~ s|LINK =	\$\(CC\)|LINK =	g++|;
    $lines =~ s|-lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz|-lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz -lmsgpack_rpc_client -lmsgpack-rpc -lmpio -lmsgpack|;

    system ("mv $file_path $file_path.back");

    open my $out, ">", $file_path;
    print $out $lines;
    close $out;
}

&main();
