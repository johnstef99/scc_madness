use std::{
    fmt::{self, Formatter},
    fs::File,
    io::{self, BufRead},
    path::Path,
    time::Instant,
};

#[derive(Debug)]
#[allow(dead_code)]
pub struct CSX {
    pub com: Vec<usize>,
    pub unc: Vec<usize>,
}

#[derive(Debug)]
pub enum CSXError {
    CouldNotOpenFile,
    BadMatrixFile,
    ErrorInHeader,
    ErrorInLine(usize),
}

impl std::error::Error for CSXError {}

impl fmt::Display for CSXError {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            CSXError::CouldNotOpenFile => write!(f, "Could not open file."),
            CSXError::BadMatrixFile => write!(f, "Bad matrix file."),
            CSXError::ErrorInHeader => write!(f, "Error in header."),
            CSXError::ErrorInLine(l) => {
                write!(f, "There is an error in line {} after the header.", l)
            }
        }
    }
}

impl CSX {
    pub fn new(num_of_vertices: usize, num_of_edges: usize) -> Self {
        CSX {
            com: Vec::with_capacity(num_of_vertices + 1),
            unc: Vec::with_capacity(num_of_edges),
        }
    }

    pub fn from_file(filename: &str) -> Result<Self, CSXError> {
        log::trace!("Reading matrix from file: {}", filename);
        let start = Instant::now();
        // open file
        if let Ok(mut lines) = read_lines(filename) {
            // skip header
            let (m, nnz) = 'header: loop {
                match lines.next() {
                    Some(Ok(header_line)) => {
                        if header_line.len() == 0 || header_line.starts_with("%") {
                            continue 'header;
                        }

                        let mut sizes = header_line
                            .split_whitespace()
                            .filter_map(|s| s.parse::<usize>().ok());

                        let m = sizes.next().unwrap_or(0);
                        let _n = sizes.next().unwrap_or(0);
                        let nnz = sizes.next().unwrap_or(0);

                        if m == 0 || _n == 0 || nnz == 0 {
                            return Err(CSXError::ErrorInHeader);
                        }

                        break (m, nnz);
                    }
                    _ => return Err(CSXError::BadMatrixFile),
                }
            };

            // read the file in csc format
            let mut csc = CSX::new(m, nnz);
            let mut com_i = 0;
            for (line_number, line) in lines.enumerate() {
                if let Ok(line) = line {
                    if !line.starts_with("%") {
                        let mut iter = line.split_whitespace();

                        fn get_next(
                            line: Option<&str>,
                            line_number: usize,
                        ) -> Result<usize, CSXError> {
                            match line {
                                Some(s) => match s.parse::<usize>() {
                                    Ok(i) => Ok(i),
                                    Err(_) => Err(CSXError::ErrorInLine(line_number)),
                                },
                                None => Err(CSXError::ErrorInLine(line_number)),
                            }
                        }

                        // subtract 1 from value in file to convert to 0-based indexing
                        let a = match get_next(iter.next(), line_number) {
                            Ok(i) => i - 1,
                            Err(e) => return Err(e),
                        };

                        let b = match get_next(iter.next(), line_number) {
                            Ok(i) => i - 1,
                            Err(e) => return Err(e),
                        };

                        while com_i <= b {
                            csc.com.insert(com_i, line_number);
                            com_i += 1;
                        }

                        csc.unc.insert(line_number, a);
                    }
                }
            }

            log::trace!("Finished reading file in {:?}", start.elapsed());
            return Ok(csc);
        }
        return Err(CSXError::CouldNotOpenFile);
    }

    pub fn info(&self) {
        println!();
        println!("{:_^20}", "Matrix Info");
        println!("M/N: {:>15}", self.com.capacity());
        println!("NNZ: {:>15}", self.unc.capacity());
        println!();
    }

    pub fn num_of_vertices(&self) -> usize {
        self.com.capacity() - 1
    }

    pub fn num_of_edges(&self) -> usize {
        self.com.capacity()
    }
}

fn read_lines<P>(filename: P) -> std::io::Result<io::Lines<io::BufReader<File>>>
where
    P: AsRef<Path>,
{
    let file = File::open(filename)?;
    Ok(io::BufReader::new(file).lines())
}
