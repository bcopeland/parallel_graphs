#! /usr/bin/perl

my $data = "oleo/twitter_graph_max";
my %partitioners = 
(
    "rand" => "./rand.pl",
    "rr" => "./roundrobin.pl",
    "pmetis" => "metis-4.0/pmetis",
    "gmax" => "./greedy_max_cut.pl",
    "gmin" => "./greedy_min_cut.pl",
);

for (my $i=2; $i <= 4; $i++)
{
    foreach my $part (keys(%partitioners))
    {
        my $of = "$data.$part.$i";
        my $undir = "$data.undir";
        my $bin = $partitioners{$part};
        next if -e $of;

        if ($part eq "pmetis")
        {
            if (! -e $undir) {
                system("./undirect.pl $data > $undir");
            }
            system("$bin $undir $i && mv $data.part.$i $of");
        }
        else {
            system("cat $data | $bin $i > $of\n");
        }
    }
}
