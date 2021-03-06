#please dont edit this file...
package PCVS::Validate::OpenMPI;
use strict;
use warnings;
use Exporter;
use Data::Dumper;
use vars qw(@ISA @EXPORT);

@ISA = 'Exporter';
@EXPORT = qw();

my $gconf;
my $max_cores;
my $max_nodes;

sub runtime_init
{
	(my $self, $gconf) = @_;
	$max_cores = $gconf->{cluster}{max_cores_per_node};
	$max_nodes = $gconf->{cluster}{max_nodes};
}

sub runtime_fini
{
}

sub get_ifdef
{
	my ($arg, $k, @c)= @_;
	foreach (0..$#c)
	{
		return $c[$_] if($k->[$_] eq $arg);
	}

	return undef;
}

sub runtime_valid
{
	my ($self, $keys, @args) = @_;
	my ($node, $proc, $mpi, $omp, $core, $net, $sched) = ();

	$node  = get_ifdef('n_node',$keys,@args);
	$proc  = get_ifdef('n_proc',$keys,@args);
	$mpi   = get_ifdef('n_mpi' ,$keys,@args); 
	$omp   = get_ifdef('n_omp' ,$keys,@args);
	$core  = get_ifdef('n_core',$keys,@args);
	$net   = get_ifdef('net' ,$keys,@args); 
	$sched = get_ifdef('sched',$keys,@args);

	# please be sure to check if the iterator exist because the system emits
	# an undef when the iterator won't be unfolded for the current configuration
	$mpi = 1 if(!defined($mpi));
	$omp = 1 if(!defined($omp));
	$node = 1 if(!defined($node));
	# '-1' because a core can be used as MPI & OMP threads
	return 0 if((($mpi)/$node + $omp -1) > $max_cores);
	return 0 if(defined $node and defined $max_nodes and $node > $max_nodes); 
	#print "Keep N=".(defined $node ? $node : "X").
		  #" p=".(defined $proc ? $proc : "X").
		  #" t=".(defined $mpi ? $mpi : "X").
		  #" o=".(defined $omp ? $omp : "X").
		  #" c=".(defined $core ? $core : "X"). 
		  #" n=".(defined $net ? $net : "X").
		  #" m=".(defined $sched ? $sched : "X")."\n";	
	return 1;
}

1;

