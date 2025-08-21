use std::env;
use std::io::{self, BufRead, Write};

const PROGRAM_INFO: &str = "Al Klimov's Number Placer\nCopyright (C) 2013-2014  Alexander A. Klimov";

/// Error handling struct similar to C++ SystemExit
struct SystemExit {
    message: String,
}

impl SystemExit {
    fn new(message: &str) -> Self {
        Self {
            message: message.to_string(),
        }
    }
    
    fn append(&mut self, text: &str) -> &mut Self {
        self.message.push_str(text);
        self
    }
}

impl From<SystemExit> for i32 {
    fn from(err: SystemExit) -> i32 {
        eprintln!("Error: {}", err.message);
        1 // EXIT_FAILURE
    }
}

/// Main Sudoku solver struct
struct NumberPlacer {
    a: usize,           // Block width * height (total numbers per row/col/box)
    b: usize,           // Total cells (A * A)
    x: bool,            // X-Sudoku mode (diagonal constraints)
    input: usize,       // Input counter
    output: usize,      // Output counter
    fail: bool,         // Failure flag
    
    content: Vec<usize>,                    // Cell values (0 = empty)
    possibilities: Vec<Vec<bool>>,          // Possible values for each cell
    address: [Vec<Vec<usize>>; 3],         // Address mappings for rows/cols/boxes
    position: [Vec<Vec<usize>>; 3],        // Position mappings for rows/cols/boxes  
    x_address_valid: [Vec<bool>; 2],       // Valid X-diagonal positions
    x_position: [Vec<usize>; 2],           // X-diagonal position mappings
}

impl NumberPlacer {
    fn new(width: usize, height: usize, x_mode: bool) -> Result<Self, String> {
        let a = width * height;
        let b = a * a;
        
        // Check for overflow conditions like the C++ version
        if a == 0 || b == 0 || b / a != a {
            return Err("Too large Sudoku!".to_string());
        }
        
        let mut placer = NumberPlacer {
            a,
            b,
            x: x_mode,
            input: 0,
            output: 0,
            fail: false,
            content: vec![0; b],
            possibilities: vec![vec![true; a]; b],
            address: [vec![vec![0; b]; 2], vec![vec![0; b]; 2], vec![vec![0; b]; 2]],
            position: [vec![vec![0; a]; a], vec![vec![0; a]; a], vec![vec![0; a]; a]],
            x_address_valid: [vec![false; b], vec![false; b]],
            x_position: [vec![0; a], vec![0; a]],
        };
        
        // Initialize position and address mappings
        for i in 0..3 {
            for j in 0..a {
                for k in 0..a {
                    let pos = match i {
                        2 => {
                            // Box positions (like C++ version)
                            (j / height) * a * height + (k / width) * a + (j % height) * width + (k % width)
                        }
                        1 => k * a + j,  // Column positions
                        _ => j * a + k,   // Row positions
                    };
                    placer.position[i][j][k] = pos;
                    placer.address[i][0][pos] = j;
                    placer.address[i][1][pos] = k;
                }
            }
        }
        
        // Initialize X-diagonal mappings if enabled
        if x_mode {
            for i in 0..2 {
                for j in 0..a {
                    let pos = j * a + if i == 1 { a - 1 - j } else { j };
                    placer.x_position[i][j] = pos;
                    placer.x_address_valid[i][pos] = true;
                }
            }
        }
        
        Ok(placer)
    }
    
    /// Add a number to the puzzle (equivalent to operator<<)
    fn add_number(&mut self, value: usize) {
        self.set_number(self.input, value);
        self.input += 1;
    }
    
    /// Get next number from solution (equivalent to operator>>)
    fn get_number(&mut self) -> usize {
        let result = self.content[self.output];
        self.output += 1;
        result
    }
    
    /// Set a number at specific position
    fn set_number(&mut self, pos: usize, value: usize) {
        self.content[pos] = value;
        for i in 0..self.a {
            self.possibilities[pos][i] = if value == 0 { true } else { i == value - 1 };
        }
    }
    
    /// Count filled cells
    fn count_filled(&self) -> usize {
        self.content.iter().filter(|&&x| x != 0).count()
    }
    
    /// Count possibilities for a cell
    fn count_possibilities(&self, pos: usize) -> u8 {
        let mut count = 0u8;
        for &possible in &self.possibilities[pos] {
            if possible {
                count += 1;
                if count > 1 {
                    break;
                }
            }
        }
        count
    }
    
    /// Check if current state is valid
    fn check(&self) -> bool {
        // Check rows, columns, and boxes
        for i in 0..3 {
            for j in 0..self.a {
                for k in 0..self.a - 1 {
                    let pos1 = self.position[i][j][k];
                    if self.content[pos1] != 0 {
                        for m in k + 1..self.a {
                            let pos2 = self.position[i][j][m];
                            if self.content[pos1] == self.content[pos2] {
                                return false;
                            }
                        }
                    }
                }
            }
        }
        
        // Check X-diagonals if enabled
        if self.x {
            for i in 0..2 {
                for k in 0..self.a - 1 {
                    let pos1 = self.x_position[i][k];
                    if self.content[pos1] != 0 {
                        for m in k + 1..self.a {
                            let pos2 = self.x_position[i][m];
                            if self.content[pos1] == self.content[pos2] {
                                return false;
                            }
                        }
                    }
                }
            }
        }
        
        true
    }
    
    /// Test if placing a number at position is valid
    fn test(&self, pos: usize) -> bool {
        let value = self.content[pos];
        if value == 0 {
            return true;
        }
        
        // Check constraints for rows, columns, and boxes
        for constraint in 0..3 {
            let row = self.address[constraint][0][pos];
            for col in 0..self.a {
                if col != self.address[constraint][1][pos] {
                    let other_pos = self.position[constraint][row][col];
                    if self.content[other_pos] == value {
                        return false;
                    }
                }
            }
        }
        
        // Check X-diagonal constraints
        if self.x {
            for diagonal in 0..2 {
                if let Some(diag_pos) = self.x_address(pos, diagonal) {
                    for col in 0..self.a {
                        if col != diag_pos {
                            let other_pos = self.x_position[diagonal][col];
                            if self.content[other_pos] == value {
                                return false;
                            }
                        }
                    }
                }
            }
        }
        
        true
    }
    
    /// Get X-diagonal address if valid
    fn x_address(&self, pos: usize, diagonal: usize) -> Option<usize> {
        if !self.x_address_valid[diagonal][pos] {
            return None;
        }
        
        Some(if diagonal == 1 {
            pos / (self.a - 1) - 1
        } else {
            pos / (self.a + 1)
        })
    }
    
    /// Modify possibilities for a cell
    fn mod_number(&mut self, pos: usize, value: usize) -> bool {
        if value == 0 || value > self.a {
            return false;
        }
        
        let idx = value - 1;
        let was_possible = self.possibilities[pos][idx];
        
        if was_possible {
            self.possibilities[pos][idx] = false;
            let count = self.count_possibilities(pos);
            
            if count == 0 {
                self.fail = true;
            } else if count == 1 {
                // Find the only remaining possibility
                for (i, &possible) in self.possibilities[pos].iter().enumerate() {
                    if possible {
                        self.content[pos] = i + 1;
                        break;
                    }
                }
            }
        }
        
        was_possible
    }
    
    /// Main solving function (equivalent to operator())
    fn solve(&mut self) -> bool {
        self.input = 0;
        self.output = 0;
        
        // Initial checks
        let filled_count = self.count_filled();
        if filled_count == 0 {
            return false;
        }
        if !self.check() {
            return false;
        }
        if filled_count == self.b {
            return true;
        }
        
        self.fail = false;
        
        // Constraint propagation
        loop {
            let mut progress = false;
            
            // Process regular constraints (rows, columns, boxes)
            for constraint in 0..3 {
                for group in 0..self.a {
                    for pos_in_group in 0..self.a {
                        let cell_pos = self.position[constraint][group][pos_in_group];
                        if self.content[cell_pos] == 0 {
                            // Eliminate values already used in this group
                            for other_pos in 0..self.a {
                                if other_pos != pos_in_group {
                                    let other_cell = self.position[constraint][group][other_pos];
                                    if self.content[other_cell] != 0 {
                                        if self.mod_number(cell_pos, self.content[other_cell]) {
                                            if self.is_done() {
                                                return self.finalize();
                                            }
                                            progress = true;
                                        }
                                    }
                                }
                            }
                            
                            // Check for hidden singles
                            for value in 1..=self.a {
                                if self.possibilities[cell_pos][value - 1] {
                                    let mut can_place_elsewhere = false;
                                    for other_pos in 0..self.a {
                                        if other_pos != pos_in_group {
                                            let other_cell = self.position[constraint][group][other_pos];
                                            if self.possibilities[other_cell][value - 1] {
                                                can_place_elsewhere = true;
                                                break;
                                            }
                                        }
                                    }
                                    
                                    if !can_place_elsewhere {
                                        self.set_number(cell_pos, value);
                                        if self.is_done() {
                                            return self.finalize();
                                        }
                                        progress = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // Process X-diagonal constraints
            if self.x {
                for diagonal in 0..2 {
                    for pos_in_diag in 0..self.a {
                        let cell_pos = self.x_position[diagonal][pos_in_diag];
                        if self.content[cell_pos] == 0 {
                            // Eliminate values already used in this diagonal
                            for other_pos in 0..self.a {
                                if other_pos != pos_in_diag {
                                    let other_cell = self.x_position[diagonal][other_pos];
                                    if self.content[other_cell] != 0 {
                                        if self.mod_number(cell_pos, self.content[other_cell]) {
                                            if self.is_done() {
                                                return self.finalize();
                                            }
                                            progress = true;
                                        }
                                    }
                                }
                            }
                            
                            // Check for hidden singles on diagonal
                            for value in 1..=self.a {
                                if self.possibilities[cell_pos][value - 1] {
                                    let mut can_place_elsewhere = false;
                                    for other_pos in 0..self.a {
                                        if other_pos != pos_in_diag {
                                            let other_cell = self.x_position[diagonal][other_pos];
                                            if self.possibilities[other_cell][value - 1] {
                                                can_place_elsewhere = true;
                                                break;
                                            }
                                        }
                                    }
                                    
                                    if !can_place_elsewhere {
                                        self.set_number(cell_pos, value);
                                        if self.is_done() {
                                            return self.finalize();
                                        }
                                        progress = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            if !progress {
                break;
            }
        }
        
        // Backtracking search
        for pos in 0..self.b {
            self.content[pos] = 0;
        }
        
        self.backtrack(0)
    }
    
    fn is_done(&self) -> bool {
        self.fail || self.count_filled() == self.b
    }
    
    fn finalize(&mut self) -> bool {
        if !self.fail {
            self.fail = !self.check();
        }
        !self.fail
    }
    
    fn backtrack(&mut self, pos: usize) -> bool {
        if pos >= self.b {
            return self.finalize();
        }
        
        for value in 1..=self.a {
            self.content[pos] = value;
            if self.possibilities[pos][value - 1] && self.test(pos) {
                if self.backtrack(pos + 1) {
                    return true;
                }
            }
        }
        
        self.content[pos] = 0;
        false
    }
}

/// Parse string to usize with validation
fn parse_size_t(s: &str) -> Result<usize, String> {
    if s.is_empty() {
        return Err(format!("\"{}\"", s));
    }
    
    for c in s.chars() {
        if !c.is_ascii_digit() {
            return Err(format!("\"{}\"", s));
        }
    }
    
    s.parse().map_err(|_| format!("\"{}\"", s))
}

/// Get number of digits needed to represent a number
fn uint_digits(n: usize) -> usize {
    if n == 0 {
        return 1;
    }
    let mut digits = 0;
    let mut num = n;
    while num > 0 {
        digits += 1;
        num /= 10;
    }
    digits
}

/// Escape string for display (simplified version of C++ repr)
fn repr(s: &str) -> String {
    format!("\"{}\"", s.escape_debug())
}

fn main() {
    let result = run_main();
    if let Err(exit_code) = result {
        std::process::exit(exit_code);
    }
}

fn run_main() -> Result<(), i32> {
    let args: Vec<String> = env::args().collect();
    
    eprintln!("{}\n", PROGRAM_INFO);
    
    // Parse command line arguments
    if args.len() > 4 {
        let err = SystemExit::new(&format!("'{}' takes at most 3 command-line arguments ({} given)", 
                                               args[0], args.len() - 1));
        return Err(err.into());
    }
    
    let mut sudoku_x = false;
    let mut width = 3;
    let mut height = 3;
    
    // Check for X-Sudoku flag
    if args.len() > 1 && (args[args.len() - 1] == "X" || args[args.len() - 1] == "x") {
        sudoku_x = true;
    } else if args.len() == 4 {
        let mut err = SystemExit::new(&format!("Invalid command-line argument (3): {}\n", repr(&args[3])));
        err.append("Must be \"X\" or \"x\"!");
        return Err(err.into());
    }
    
    // Parse width and height
    let max_args = if sudoku_x { args.len() - 1 } else { args.len() };
    
    for i in 1..std::cmp::min(3, max_args) {
        let value = match parse_size_t(&args[i]) {
            Ok(v) if v >= 2 => v,
            _ => {
                let mut err = SystemExit::new(&format!("Invalid command-line argument ({}): {}\n", 
                                                       i, repr(&args[i])));
                err.append(&format!("Must be an integer (2 <= n <= {})", usize::MAX));
                if !sudoku_x && args.len() - 1 == i {
                    err.append(", \"X\" or \"x\"!");
                } else {
                    err.append("!");
                }
                return Err(err.into());
            }
        };
        
        if i == 1 {
            width = value;
        } else {
            height = value;
        }
    }
    
    // Calculate sizes
    let sudoku_size = width * height;
    let total_cells = sudoku_size * sudoku_size;
    let digit_width = uint_digits(sudoku_size);
    let line_length = digit_width * total_cells;
    
    // Check for overflow
    if sudoku_size == 0 || total_cells == 0 || total_cells / sudoku_size != sudoku_size ||
       line_length / total_cells != digit_width {
        let err = SystemExit::new("Too large Sudoku!");
        return Err(err.into());
    }
    
    eprintln!("Sudoku size: {}x{} ({}x{})", width, height, sudoku_size, sudoku_size);
    eprintln!("X-Sudoku: {}\n", if sudoku_x { "yes" } else { "no" });
    eprint!("Preparing... ");
    io::stderr().flush().map_err(|_| -> i32 { SystemExit::new("IO error").into() })?;
    
    let mut sudoku = NumberPlacer::new(width, height, sudoku_x)
        .map_err(|e| -> i32 { SystemExit::new(&e).into() })?;
    
    eprintln!("done.");
    
    let stdin = io::stdin();
    let mut first_line = true;
    
    for line_result in stdin.lock().lines() {
        let line = match line_result {
            Ok(l) => l,
            Err(_) => break,
        };
        
        if line.is_empty() {
            if first_line {
                eprintln!("EOF @ first line -- nothing to do.");
                break;
            } else {
                eprintln!("Warning: Ignoring blank line");
                first_line = false;
                continue;
            }
        }
        
        if line.len() != line_length {
            let err = SystemExit::new(&format!("Invalid input!\nEach line must be {} characters long!", line_length));
            return Err(err.into());
        }
        
        // Parse input
        for i in 0..total_cells {
            let start = i * digit_width;
            let end = start + digit_width;
            let number_str = &line[start..end];
            
            let number = match parse_size_t(number_str) {
                Ok(n) if n <= sudoku_size => n,
                _ => {
                    let err = SystemExit::new(&format!("Invalid input: {}\nMust be an integer (0 <= n <= {})!", 
                                repr(number_str), sudoku_size));
                    return Err(err.into());
                }
            };
            
            sudoku.add_number(number);
        }
        
        // Solve and output
        if sudoku.solve() {
            for _ in 0..total_cells {
                let number = sudoku.get_number();
                print!("{:0width$}", number, width = digit_width);
            }
        } else {
            for _ in 0..total_cells {
                print!("{:0width$}", 0, width = digit_width);
            }
        }
        println!();
        
        if first_line {
            first_line = false;
        }
    }
    
    eprintln!("End of file.");
    Ok(())
}