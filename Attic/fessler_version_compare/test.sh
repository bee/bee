#/bin/bash


echo -n "testing usage .. "
if ! ./version_compare >/dev/null ; then
    echo "OK"
else
    echo "FAILED"
fi


for p in --lt --gt --le --ge --eq --ne ; do 
   echo -n "testing fail when no args in ${p} .. "
   if ! ./version_compare ${p} >/dev/null ; then
       echo "OK"
   else
       echo "FAILED"
   fi
   
   echo -n "testing fail when no args in ${p} .. "
   if ! ./version_compare ${p} dummy-1.2-3 >/dev/null ; then
       echo "OK"
   else
       echo "FAILED"
   fi
done


P1=paket-1.2.3-4
P2=paket-1.2.5-4

lt() {
    echo -n "  $1 --lt $2 "
    if ./version_compare --lt $1 $2 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $2 --lt $1 "
    if ! ./version_compare --lt $2 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $1 --lt $1 "
    if ! ./version_compare --lt $1 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi
}


le() {  # $1 < $2
    echo -n "  $1 --le $2 "
    if ./version_compare --le $1 $2 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $2 --le $1 "
    if ! ./version_compare --le $2 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $1 --le $1 "
    if ./version_compare --le $1 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi
}

gt() { 
    echo -n "  $1 --gt $2 "
    if ! ./version_compare --gt $1 $2 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $2 --gt $1 "
    if  ./version_compare --gt $2 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $1 --gt $1 "
    if ! ./version_compare --gt $1 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi
}



ge() {  
    echo -n "  $1 --ge $2 "
    if ! ./version_compare --ge $1 $2 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $2 --ge $1 "
    if  ./version_compare --ge $2 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $1 --ge $1 "
    if ./version_compare --ge $1 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi
}

eq() {  
    echo -n "  $1 --eq $2 "
    if ! ./version_compare --eq $1 $2 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $2 --eq $1 "
    if  ! ./version_compare --eq $2 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n " $1 --eq $1 "
    if ./version_compare --eq $1 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi
}

ismax() {  
    echo -n "  $1 --ismax $2 "
    if ! ./version_compare --ismax $1 $2 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $2 --ismax $1 "
    if  ./version_compare --ismax $2 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n " $1 --ismax $1 "
    if ./version_compare --ismax $1 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi
}

ismin() {  
    echo -n "  $1 --ismin $2 "
    if  ./version_compare --ismin $1 $2 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n "! $2 --ismin $1 "
    if  ! ./version_compare --ismin $2 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi

    echo -n " $1 --ismax $1 "
    if  ./version_compare --ismin $1 $1 >/dev/null ; then
        echo "OK"
    else
        echo "FAIL"
    fi
}

for i in \
  "paket-1.2.3-4 paket-1.2.5-4" \
  "paket-1.2.3_alpha-0 paket-1.2.3_beta-0" \
   "paket-1.2.3_alpha-0 paket-1.2.3_alpha1-0" \
   "paket-1.2.3_beta-0 paket-1.2.3_beta1-0" \
   "paket-1.2.3_rc-0 paket-1.2.3_rc1-0" \
   "paket-1.2.3-0 paket-1.2.3_p-0" \
   "paket-1.2.3_p-0 paket-1.2.3_p1-0" \
   "paket-1-0 paket-1a_p-0" \
   "paket-a-0 paket-1_p-0" \
   "paket-1-0 paket-1.0_p-0" \
   "paket-1.0-0 pake-1a.0_p-0" \
   "paket_1-0-0 paket-0_p-0" \
   "1.0-0 paket_sub-10_p-0" \
   "a_paket_subname-1.3.4-2 a_paket_subname-123456_p-0" \
   "1.a 1.0_p" \
   "1.a 1.0_p-0" 
   
  do
    lt ${i}
    le ${i}
    gt ${i}
    ge ${i}
    ismax ${i}
    ismin ${i}
done
