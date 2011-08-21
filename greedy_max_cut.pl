#!/usr/bin/env perl

use strict;

# greedy algorithm:
#  sort on degree.
#   for each node, if not in partition pick one randomly.
#     add all unassigned siblings to the other partitions.
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

my $proc_id = 0;
foreach my $vertex (sort {
        scalar(@{$siblings{$b}}) <=> scalar(@{$siblings{$a}})
    } keys(%siblings))
{
    if (!defined($parts[$vertex]))
    {
        $parts[$vertex] = $proc_id;
    }
    # now assign all the siblings to the other partitions in turn
    my $tmp_proc_id = ($proc_id + 1) % $nproc;
    foreach my $child (@{$siblings{$vertex}})
    {
        if (!defined($parts[$child]))
        {
            $parts[$child] = $tmp_proc_id;
            do {
                $tmp_proc_id = ($tmp_proc_id + 1) % $nproc;
            } while ($tmp_proc_id == $proc_id);
        }
    }
    $proc_id = ($proc_id + 1) % $nproc;
    $siblings{$vertex} = undef;
}

foreach my $part (@parts)
{
    print "$part\n";
}
