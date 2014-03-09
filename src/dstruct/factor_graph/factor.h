
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

  class Factor {
  public:
    long id;
    long weight_id;
    int n_variables;

    long n_start_i_vif;

    std::vector<VariableInFactor> tmp_variables;
    int func_id; 

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
    inline double _potential_imply(const VariableInFactor * const vifs,
                                   const double * const var_values, 
                                   const long &, const double &) const;

    inline double potential(const VariableInFactor * const vifs,
      const double * const var_values,
      const long & vid, const double & proposal) const{ 
      
      if(func_id == 0){
        return _potential_imply(vifs, var_values, vid, proposal);
      }else if(func_id == 4){
        return _potential_imply(vifs, var_values, vid, proposal);
      }else{
        std::cout << "Unsupported Factor Function ID= " << func_id << std::endl;
        assert(false);
      }
      return 0.0;
    }
 
  };

  inline double dd::Factor::_potential_imply(
      const VariableInFactor * const vifs,
      const double * const var_values, 
      const long & vid, const double & proposal) const{

      double sum = 0.0;
      for(long i_vif=n_start_i_vif;i_vif<n_start_i_vif+n_variables;i_vif++){
        const VariableInFactor & vif = vifs[i_vif];
        if(vif.n_position == n_variables - 1){
          if(vif.vid == vid){
            sum += (vif.is_positive == true ? proposal : 1-proposal);
          }else{
            sum += (vif.is_positive == true ? var_values[vif.vid]
              : 1-var_values[vif.vid]);
          }
        }else{
          if(vif.vid == vid){
            sum += (vif.is_positive == false ? proposal : 1-proposal);
          }else{
            sum += (vif.is_positive == false ? var_values[vif.vid]
              : var_values[vif.vid]);
          }
        }
      }
      if(sum != 0){
        return 1.0;
      }else{
        return 0.0;
      }
  }
}

#endif




