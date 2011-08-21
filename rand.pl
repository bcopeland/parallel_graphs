#!/usr/bin/env perl

srand(120);
my $nproc = shift @ARGV;

while(<>)
{
    $x = int(rand($nproc));
    print "$x\n";
}
