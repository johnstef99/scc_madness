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

fn main() {
    let args = Cli::parse();

    env_logger::Builder::new()
        .filter_level(args.verbose.log_level_filter())
        .init();

    let filename = args.filename.as_str();

    let mut graph = Graph::from_file(filename);

    graph.csc.info();
    graph.trim();
    graph.color_scc();

    log::info!("Number of scc: {}", graph.num_of_scc());
}
