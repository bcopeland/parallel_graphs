#! /usr/bin/perl

use strict;

my $line = <>;
my ($verts, $edges) = split(/ /, $line);

my $id = 1;

my %connect;
while(<>)
{
    chomp;
    my @edges = split(/ /, $_);
    foreach my $neighbor (@edges)
    {
        $connect{$id}->{$neighbor} = 1;
        $connect{$neighbor}->{$id} = 1;
    }
    $id++;
}

my $fn = "undirect.tmp";
open (TMP, ">$fn");
my $edges = 0;
for (my $id=1; $id <= $verts; $id++)
{
    my $last_n;
    foreach my $n (sort {$a <=> $b} keys(%{$connect{$id}}))
    {
        next if ($n == $last_n || $id == $n);
        $last_n = $n;
        print TMP $n, " ";
        $edges++;
    }
    print TMP "\n";
}
close(TMP);

$edges /= 2;

print "$verts $edges\n";
open(TMP, $fn);
while(<TMP>)
{
    print;
}
close(TMP);
unlink($fn);



