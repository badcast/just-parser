# JNOParser
 Just Node Object Parser

//Syntax    view 
//Comment ...                 this is comment line 
node_name                   //this is node name
{                           //this is structure nodes
    property_name   "value"   //this is property name-property_name and "value"-value for property
                              //property types:
                                //string:  "this is string" 
                                //number:  -18446744073709551615 to 18446744073709551615 (64 bits) - 2^64-1 
                                //real:    -inf to +inf                                  find minimum value double at->https://stackoverflow.com/questions/1153548/minimum-double-value-in-c-c
                                //boolean: true and false                                logical value
                                
    node_2{                 //Recursive node
        n2_word     "Hello, "
        node_3{
            n3_word "world!"
        }
    }
}
