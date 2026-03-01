#pragma once

// For pointer parameters passed to functions.
// Singals that the function takes ownership of the pointer.
// Using the pointer after passing to the function
// causes udnefined behaviour
#define TRANSFER

// Returns non-owning pointer
#define NONOWNING