mod csx;
mod graph;

use graph::Graph;

use clap::Parser;
use clap_verbosity_flag::{InfoLevel, Verbosity};

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

    let graph = Graph::from_file(filename);

    graph.csc.info();

    println!("{:?}", graph);
}
