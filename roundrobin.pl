#!/usr/bin/env perl

my $nproc = shift @ARGV;
my $header = <>;
my ($verts, $edges) = split(/ /, $header);

my $count = 0;
my $proc_id = 0;
while(<>)
{
    print "$proc_id\n";
    $proc_id = ($proc_id + 1) % $nproc;
}
