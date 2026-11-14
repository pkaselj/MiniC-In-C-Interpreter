# Returns b squared in m-modulo arithmetic
function modsqr(b, m)
{
    return (b * b) % m;
}

# Efficient calculation of b to the power of n in m-modulo arithmetic 
function modexp(b, n, m)
{
    if(n == 1)
    {
        return b;
    }
    else
    {
        return (modsqr(b, m) * modexp(b, n - 1, m)) % m;
    }
}

function main()
{
    return modexp(5, 3, 4);
}

main();