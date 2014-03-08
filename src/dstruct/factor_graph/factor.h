
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
    std::vector<VariableInFactor> variables;
    int func_id; 
    Factor(const long & _id,
           const int & _weight_id,
           const int & _func_id,
           const int & _n_variables){
      this->id = _id;
      this->weight_id = _weight_id;
      this->func_id = _func_id;
      this->n_variables = _n_variables;
    }
    template<bool does_change_evid>
    inline double _potential_imply(const std::vector<Variable> &, 
                                   const long &, const double &) const;

    template<bool does_change_evid>
    inline double potential(const std::vector<Variable> & _variables,
      const long & vid, const double & proposal) const{ 
      
      if(func_id == 0){
        return this->template _potential_imply<does_change_evid>(_variables, vid, proposal);
      }else if(func_id == 4){
        return this->template _potential_imply<does_change_evid>(_variables, vid, proposal);
      }else{
        std::cout << "Unsupported Factor Function ID= " << func_id << std::endl;
        assert(false);
      }
      return 0.0;
    }
 
  };

  template<bool does_change_evid>
  inline double dd::Factor::_potential_imply(const std::vector<Variable> & _variables, 
      const long & vid, const double & proposal) const{

      double sum = 0.0;
      for(int i=0;i<n_variables;i++){
        const VariableInFactor & vif = variables[i];
        if(vif.n_position == n_variables - 1){
          if(vif.vid == vid){
            sum += (vif.is_positive == true ? proposal : 1-proposal);
          }else{
            sum += (vif.is_positive == true ? get_vassign<does_change_evid>(_variables[vif.vid]) 
              : 1-get_vassign<does_change_evid>(_variables[vif.vid]));
          }
        }else{
          if(vif.vid == vid){
            sum += (vif.is_positive == false ? proposal : 1-proposal);
          }else{
            sum += (vif.is_positive == false ? get_vassign<does_change_evid>(_variables[vif.vid]) 
              : 1-get_vassign<does_change_evid>(_variables[vif.vid]));
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




