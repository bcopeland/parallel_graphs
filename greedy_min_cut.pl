#!/usr/bin/env perl

use strict;

# greedy algorithm:
#  sort on minimum degree.
#   for each node, if not in partition pick one randomly.
#     add all siblings to the same partition.
my $nproc = shift @ARGV;
my $header = <>;
my ($verts, $edges) = split(/ /, $header);

my $count = 0;

my %siblings = ();
my @parts;
while(<>)
{
    my $v = $count++;
    my @neighbors = split(/ /);

    # count indegree for our neighbors
    foreach my $n (@neighbors)
    {
        push @{$siblings{$n}}, $v;
    }
    push @{$siblings{$v}}, @neighbors;
}

my $max_ct = $verts/$nproc;
my $proc_id = 0;
my $in_proc = 0;
foreach my $vertex (sort {
        scalar(@{$siblings{$a}}) <=> scalar(@{$siblings{$b}})
    } keys(%siblings))
{
    # recursively assign this and all siblings to the same partition
    # until the partition is full
    my @queue = ();
    if (!defined($parts[$vertex]))
    {
        push @queue, $vertex;
    }
    while (@queue)
    {
        my $x = pop @queue;
        if ($in_proc >= $max_ct)
        {
            $proc_id = ($proc_id + 1) % $nproc;
            $in_proc = 0;
        }
        $parts[$x] = $proc_id;
        $in_proc++;
        foreach my $child (@{$siblings{$x}})
        {
            if (!defined($parts[$child]))
            {
                push @queue, $child;
            }
        }
        $siblings{$x} = undef;
    }
}

foreach my $part (@parts)
{
    print "$part\n";
}
