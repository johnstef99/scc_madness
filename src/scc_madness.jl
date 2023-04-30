module scc_madness

using Printf
using MatrixMarket
using Graphs

if (length(ARGS) != 1)
  println("Usage: julia scc_madness.jl <MatrixMarketFile>")
  exit(1)
end

filename::String = ARGS[1]
@info "Start reading file"
time = @elapsed M = MatrixMarket.mmread(filename)
time += @elapsed g = SimpleDiGraph(M)
@info "Done reading file in $time"

N = 3
total_time = 0
for i in 1:N
  @info "Start computing SCCs $i/$N"
  time = @elapsed sccs = strongly_connected_components(g)
  @info("SCCs found: $(length(sccs)) in $time")
  global total_time += time
end
avg_time = total_time / N


@printf("%-30s %.3f\n", last(split(filename, '/')), avg_time)

end # module scc_madness
