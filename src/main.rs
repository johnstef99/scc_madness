use clap::Parser;
use clap_verbosity_flag::{InfoLevel, Verbosity};

use scc_madness::graph::Graph;

#[derive(Parser, Debug)]
#[command(author, about, version)]
struct Cli {
    /// Path to matrix market file
    #[clap(short, long)]
    filename: String,

    #[command(flatten)]
    verbose: Verbosity<InfoLevel>,
}

fn main() -> std::io::Result<()> {
    let args = Cli::parse();

    env_logger::Builder::new()
        .filter_level(args.verbose.log_level_filter())
        .init();

    let filename = args.filename.as_str();

    let mut graph1 = Graph::from_file(filename);
    graph1.csc.info();
    graph1.trim_par()?;
    graph1.color_scc();
    log::info!("Number of scc: {}", graph1.num_of_scc());
    std::mem::drop(graph1);

    let mut graph2 = Graph::from_file(filename);
    graph2.trim_par()?;
    graph2.color_scc_par();
    log::info!("Number of scc: {}", graph2.num_of_scc());
    std::mem::drop(graph2);

    Ok(())
}
