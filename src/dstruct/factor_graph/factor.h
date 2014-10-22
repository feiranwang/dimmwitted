
#ifndef _FACTOR_H_
#define _FACTOR_H_


namespace dd{

  template<bool does_change_evid>
  inline const double & get_vassign(const Variable & v);

  template<>
  inline const double & get_vassign<true>(const Variable & v){
    return v.assignment_free;
  }

  template<>
  inline const double & get_vassign<false>(const Variable & v){
    return v.assignment_evid;
  }

  class CompactFactor{
  public:
    long id;
    int func_id; 
    int n_variables;
    long n_start_i_vif;

    CompactFactor(){}

    CompactFactor(const long & _id){
      id = _id;
    }

    inline double _potential_or(const VariableInFactor * const vifs,
                                   const double * const var_values, 
                                   const long &, const double &) const;


    inline double _potential_and(const VariableInFactor * const vifs,
                                   const double * const var_values, 
                                   const long &, const double &) const;


    inline double _potential_equal(const VariableInFactor * const vifs,
                                   const double * const var_values, 
                                   const long &, const double &) const;


    inline double _potential_imply(const VariableInFactor * const vifs,
                                   const double * const var_values, 
                                   const long &, const double &) const;
    
    inline double _potential_multinomial(const VariableInFactor * const vifs,
                                   const double * const var_values, 
                                   const long &, const double &) const;

    inline double _potential_tree(const VariableInFactor * const vifs, 
      const double * const var_values, const long &, const double &) const;

    inline double _potential_parentlabel(const VariableInFactor * const vifs, 
      const double * const var_values, const long &, const double &) const;

    inline double potential(const VariableInFactor * const vifs,
      const double * const var_values,
      const long & vid, const double & proposal) const{ 
      
      if(func_id == 0){
        return _potential_imply(vifs, var_values, vid, proposal);
      }else if(func_id == 4){
        return _potential_imply(vifs, var_values, vid, proposal);
      }else if(func_id == 1){ // OR
        return _potential_or(vifs, var_values, vid, proposal);
      }else if(func_id == 2){ // AND
        return _potential_and(vifs, var_values, vid, proposal);   
      }else if(func_id == 3){ // EQUAL
        return _potential_equal(vifs, var_values, vid, proposal);     
      } else if (func_id == 5 || func_id == 8) { // multinomial
        return _potential_multinomial(vifs, var_values, vid, proposal);
      } else if (func_id == 6) { // tree constraint
        return _potential_tree(vifs, var_values, vid, proposal);
      } else if (func_id == 7) { // parent label constraint
        return _potential_parentlabel(vifs, var_values, vid, proposal);
      } else {
        std::cout << "Unsupported Factor Function ID= " << func_id << std::endl;
        assert(false);
      }
      return 0.0;
    }

  };

  class Factor {
  public:
    long id;
    long weight_id;
    int func_id;
    int n_variables;

    long n_start_i_vif;

    std::vector<VariableInFactor> tmp_variables;

    bool is_sampled;

    Factor(){

    }

    Factor(const long & _id,
           const int & _weight_id,
           const int & _func_id,
           const int & _n_variables){
      this->id = _id;
      this->weight_id = _weight_id;
      this->func_id = _func_id;
      this->n_variables = _n_variables;
    }
 
  };

  // parser: factor for enforcing parent label consistency
  // variables: root label, left child's parent label, right child's parent label, tree pointer
  // return 1 if this subtree is active and (left child's parent label != root label 
  // or right child's parent label != root)
  inline double dd::CompactFactor::_potential_parentlabel(const VariableInFactor * vifs, 
    const double * var_values, const long & vid, const double & proposal) const {
    int val = 0;
    int active = 0;
    int sum = 0;

    // check active, get the value of one other variable
    for (long i_vif = n_start_i_vif; i_vif < n_start_i_vif + n_variables; i_vif++) {
      const VariableInFactor & vif = vifs[i_vif]; 
      if (vif.n_position == n_variables - 1) {
        if (vif.vid == vid) {
          active = (proposal == vif.equal_to);
        } else {
          active = (var_values[vif.vid] == vif.equal_to);
        }
      } else {
        if (vif.vid == vid) {
          val = proposal;
        } else {
          val = var_values[vif.vid];
        }
        break;
      }
    }

    if (!active) return 0.0;

    // check equality
    for (long i_vif = n_start_i_vif; i_vif < n_start_i_vif + n_variables; i_vif++) {
      const VariableInFactor & vif = vifs[i_vif]; 
      if (vif.n_position != n_variables - 1) {
        if (vif.vid == vid) {
          sum += (proposal != val);
        } else {
          sum += (var_values[vif.vid] != val);
        }
      }
    }

    return sum != 0 ? 1.0 : 0.0;

    // // pointer
    // const VariableInFactor& vifn = vifs[n_start_i_vif + n_variables - 1];
    // if (vifn.vid == vid) {
    //   sum += (proposal == vifn.equal_to);
    // } else {
    //   sum += (var_values[vifn.vid] == vifn.equal_to);
    // }
    // // active
    // if (sum != 0) {
    //   sum = 0;
    //   const VariableInFactor& vif0 = vifs[n_start_i_vif];
    //   // check whether the variables between equal to the first variable
    //   for (long i_vif = n_start_i_vif + 1; i_vif < n_start_i_vif + n_variables - 1; i_vif++) {
    //     const VariableInFactor& vif = vifs[i_vif];
    //     if (vif.vid == vid) {
    //       sum += 1 - (proposal == var_values[vif0.vid]);
    //     } else {
    //       sum += 1 - (var_values[vif.vid] == var_values[vif0.vid]);
    //     }
    //   }
    // }

    // return sum != 0 ? 1.0 : 0.0;
  }

  // parser: factor for enforcing tree structure
  // input: pointer, pointers of all potential parents
  // return 1 if the first pointer is active but no parents are active
  // or the first pointer is inactive and exists parents that are active
  inline double dd::CompactFactor::_potential_tree(const VariableInFactor * vifs, 
    const double * var_values, const long & vid, const double & proposal) const {
    int sum = 0;
    for (long i_vif = n_start_i_vif + 1; i_vif < n_start_i_vif + n_variables; i_vif++) {
      const VariableInFactor& vif = vifs[i_vif];
      if (vif.vid == vid) {
        sum += (proposal == vif.equal_to);
      } else {
        sum += (var_values[vif.vid] == vif.equal_to);
      }
    }

    const VariableInFactor& vif = vifs[n_start_i_vif];
    // the first variable == 0
    if ((vif.vid != vid && var_values[vif.vid] == 0) || (vif.vid == vid && proposal == 0)) {
      return sum != 0 ? 1.0 : 0.0;
    } else {
      return sum != 0 ? 0.0 : 1.0;
    }
    
  }

  // potential for multinomial variable
  inline double dd::CompactFactor::_potential_multinomial(const VariableInFactor * vifs, 
    const double * var_values, const long & vid, const double & proposal) const {


    int sum = 0;
    for (long i_vif = n_start_i_vif; i_vif < n_start_i_vif + n_variables; i_vif++) {

      const VariableInFactor& vif = vifs[i_vif];
      // no predicate
      if (vif.equal_to < 0) continue;
      // check predicate
      if (vif.vid == vid) {
        sum += 1 - (proposal == vif.equal_to);
      } else {
        sum += 1 - (var_values[vif.vid] == vif.equal_to);
      }
    }

    if (sum == 0) {
      return 1.0;
    } else {
      return 0.0;
    }
  }

  inline double dd::CompactFactor::_potential_equal(
    const VariableInFactor * const vifs,
    const double * const var_values, 
    const long & vid, const double & proposal) const{

    const VariableInFactor & vif = vifs[n_start_i_vif];
    double sum;
    if(vif.vid == vid){
      sum = (vif.is_positive == true ? (proposal==vif.equal_to) : 1-(proposal==vif.equal_to));
    }else{
      sum = (vif.is_positive == true ? (var_values[vif.vid]==vif.equal_to) : 1-(var_values[vif.vid]==vif.equal_to));
    }

    for(long i_vif=n_start_i_vif+1;i_vif<n_start_i_vif+n_variables;i_vif++){
      const VariableInFactor & vif = vifs[i_vif];
      if(vif.vid == vid){
        if(sum != (vif.is_positive == true ? (proposal==vif.equal_to) : 1-(proposal==vif.equal_to))){
          return 0.0;
        }
      }else{
        if(sum != (vif.is_positive == true ? (var_values[vif.vid]==vif.equal_to) : 1-(var_values[vif.vid]==vif.equal_to))){
          return 0.0;
        }
      }
    }
    return 1.0;
  }

  inline double dd::CompactFactor::_potential_and(
    const VariableInFactor * const vifs,
    const double * const var_values, 
    const long & vid, const double & proposal) const{

    double sum = 0.0;
    for(long i_vif=n_start_i_vif;i_vif<n_start_i_vif+n_variables;i_vif++){
      const VariableInFactor & vif = vifs[i_vif];
      if(vif.vid == vid){
        sum += (vif.is_positive == false ? (proposal==vif.equal_to) : 1-(proposal==vif.equal_to));
      }else{
        sum += (vif.is_positive == false ? (var_values[vif.vid]==vif.equal_to)
          : 1-(var_values[vif.vid]==vif.equal_to));
      }
    }
    if(sum != 0){
      return 1.0 - 1.0;
    }else{
      return 1.0 - 0.0;
    }
  }

  inline double dd::CompactFactor::_potential_or(
    const VariableInFactor * const vifs,
    const double * const var_values, 
    const long & vid, const double & proposal) const{

    double sum = 0.0;
    for(long i_vif=n_start_i_vif;i_vif<n_start_i_vif+n_variables;i_vif++){
      const VariableInFactor & vif = vifs[i_vif];
      if(vif.vid == vid){
        sum += (vif.is_positive == true ? (proposal==vif.equal_to) : 1-(proposal==vif.equal_to));
      }else{
        sum += (vif.is_positive == true ? (var_values[vif.vid]==vif.equal_to)
          : 1-(var_values[vif.vid]==vif.equal_to));
      }
    }
    if(sum != 0){
      return 1.0;
    }else{
      return 0.0;
    }
  }


  inline double dd::CompactFactor::_potential_imply(
      const VariableInFactor * const vifs,
      const double * const var_values, 
      const long & vid, const double & proposal) const{

      double sum = 0.0;

      for(long i_vif=n_start_i_vif;i_vif<n_start_i_vif+n_variables;i_vif++){
        const VariableInFactor & vif = vifs[i_vif];
        
        if(vif.n_position == n_variables - 1){
          if(vif.vid == vid){
            sum += (vif.is_positive == true ? (proposal==vif.equal_to) : 1-(proposal==vif.equal_to));
          }else{
            sum += (vif.is_positive == true ? (var_values[vif.vid]==vif.equal_to)
              : 1-(var_values[vif.vid]==vif.equal_to));
          }
        }else{
          if(vif.vid == vid){
            sum += (vif.is_positive == false ? (proposal==vif.equal_to) : 1-(proposal==vif.equal_to));
          }else{
            sum += (vif.is_positive == false ? (var_values[vif.vid]==vif.equal_to)
              : (var_values[vif.vid]==vif.equal_to));
          }
        }
      }

      //if(vid == 548590 || vid == 2531610){
      //  std::cout << "@" << n_variables << "  v" << vid << "    " << proposal << "    " << sum << std::endl;  
      //}


      if(sum != 0){
        //std::cout << "f" << id << " " << proposal << " -> 1.0" << "    " << vifs[0].equal_to <<std::endl;
        return 1.0;
      }else{
        //std::cout << "f" << id << " " << proposal << " -> 0.0" << "    " << vifs[0].equal_to <<std::endl;
        return 0.0;
      }
  }
}

#endif




