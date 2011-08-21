#!/usr/bin/env perl

my $nproc = shift @ARGV;
my $header = <>;
my ($verts, $edges) = split(/ /, $header);

# compute degree centrality of each vertex, sort on that and assign
# in round-robin fashion to different clusters
my $count = 0;
my %centrality = ();
while(<>)
{
    $centrality{$count++} = ((scalar split(/ /)) / ($verts - 1));
}

my $proc_id = 0;
foreach my $vertex (sort {$centrality{$a} <=> $centrality{$b}}
                    keys(%centrality))
{
    @parts[$vertex] = $proc_id;
    $proc_id = ($proc_id + 1) % $nproc;
}

foreach $part (@parts)
{
    print "$part\n";
}
