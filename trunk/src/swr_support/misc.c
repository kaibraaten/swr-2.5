int umin( int check, int ncheck )
{
  return check < ncheck ? check : ncheck;
}

int umax( int check, int ncheck )
{
  return check > ncheck ? check : ncheck;
}

int urange( int mincheck, int check, int maxcheck )
{
  if( check < mincheck )
    return mincheck;

  if( check > maxcheck )
    return maxcheck;

  return check;
}
