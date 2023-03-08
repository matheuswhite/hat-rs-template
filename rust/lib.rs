#![no_std]
#![no_builtins]

extern crate panic_halt;
extern crate hat;
extern crate hat_macros;
extern crate alloc;

use core::time::Duration;
use hat::*;

#[hat_macros::main]
pub async fn main_task() -> TaskResult {
    loop {
        log!("Hello, World!");
        delay(Duration::from_secs(1)).await;
    }

}
