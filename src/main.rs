use std::{
    thread,
    time::{Duration, Instant},
};

use clap::Parser;
use clap_verbosity_flag::{InfoLevel, Verbosity};

use scc_madness::graph::Graph;

#[derive(Parser, Debug)]
#[command(author, about, version)]
struct Cli {
    /// Path to matrix market file
    #[clap(short, long)]
    filename: String,

    /// Number of runs
    #[clap(short, long, default_value = "3")]
    num_runs: usize,

    /// Whether to print to stdout the mean result
    #[clap(short, long)]
    print: bool,

    #[command(flatten)]
    verbose: Verbosity<InfoLevel>,
}

#[derive(Debug)]
struct BenchResult {
    filename: String,
    v: usize,
    e: usize,
    scc: usize,
    serial: Duration,
    par_safe: Duration,
    par_unsafe: Duration,
}

impl std::fmt::Display for BenchResult {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{:<25} {:>10} {:>10} {:>10} {:>15?} {:>15?} {:>15?}",
            self.filename, self.v, self.e, self.scc, self.serial, self.par_safe, self.par_unsafe
        )
    }
}

fn benchemark(filename: &str) -> std::io::Result<BenchResult> {
    let mut graph = Graph::from_file(filename);
    log::info!("Running in serial");
    let serial = Instant::now();
    graph.trim();
    graph.color_scc();
    let serial = serial.elapsed();
    log::trace!("Number of scc: {}", graph.num_of_scc());
    std::mem::drop(graph);

    log::info!("Running in parallel SAFE");
    let mut graph = Graph::from_file(filename);
    let par_safe = Instant::now();
    graph.trim_par()?;
    graph.color_scc_par();
    let par_safe = par_safe.elapsed();
    log::trace!("Number of scc: {}", graph.num_of_scc());
    std::mem::drop(graph);

    log::info!("Running in parallel UNSAFE");
    let mut graph = Graph::from_file(filename);
    let par_unsafe = Instant::now();
    unsafe {
        graph.trim_par_unsafe()?;
        graph.color_scc_par_unsafe();
    }
    let par_unsafe = par_unsafe.elapsed();
    log::trace!("Number of scc: {}", graph.num_of_scc());

    let res = BenchResult {
        filename: filename.split('/').last().unwrap().to_string(),
        e: graph.csc.num_of_edges,
        v: graph.csc.num_of_vertices,
        scc: graph.num_of_scc(),
        serial,
        par_safe,
        par_unsafe,
    };

    log::trace!("{:?}", res);

    Ok(res)
}

fn main() -> std::io::Result<()> {
    let args = Cli::parse();

    env_logger::Builder::new()
        .filter_level(args.verbose.log_level_filter())
        .init();

    let filename = args.filename.as_str();
    let number_of_runs = args.num_runs;

    log::info!(
        "Available threads: {:?}",
        thread::available_parallelism().unwrap()
    );

    let mut results: Vec<BenchResult> = vec![];
    for i in 0..number_of_runs {
        let i = format!("Run {}/{}", i + 1, number_of_runs);
        log::info!("{:_^30}", i);
        let res = benchemark(filename)?;
        results.push(res);
    }
    let len = results.len() as u32;
    let mean: BenchResult = results.iter().fold(
        BenchResult {
            scc: 0,
            v: 0,
            e: 0,
            filename: filename.to_string().split('/').last().unwrap().to_string(),
            serial: Duration::from_secs(0),
            par_safe: Duration::from_secs(0),
            par_unsafe: Duration::from_secs(0),
        },
        |acc, x| BenchResult {
            filename: acc.filename.to_string(),
            e: x.e,
            v: x.v,
            scc: x.scc,
            serial: acc.serial + x.serial / len,
            par_safe: acc.par_safe + x.par_safe / len,
            par_unsafe: acc.par_unsafe + x.par_unsafe / len,
        },
    );
    log::trace!("Mean: {:?}", mean);

    if args.print {
        println!("{}", mean);
    }

    Ok(())
}
