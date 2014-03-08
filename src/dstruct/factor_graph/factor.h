
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
    std::vector<VariableInFactor> variables;
    int func_id; 

    Factor(const long & _id,
           const int & _weight_id,
           const int & _func_id){
      this->id = _id;
      this->weight_id = _weight_id;
      this->func_id = _func_id;
    }

    template<bool does_change_evid>
    inline double potential(const std::vector<Variable> & variables, 
      const long & vid, const double & proposal) const{
      if(vid == -1){
        return get_vassign<does_change_evid>(variables[0]) == 1.0;
      }else{
        return get_vassign<does_change_evid>(variables[vid]) == 1.0;
      }
    }
 

  };

}

#endif