#include "SASPSO/Particle.hpp"

using namespace type_traits;

template <size_t dim>
void Particle<dim>::initialize()
{
    // Draw uniformly a value in the range [lower_bound_i, upper_bound_i] for each dimension i
    for (size_t i = 0; i < dim; ++i)
    {
        std::uniform_real_distribution<double> distr(problem_->get_lower_bound(i), problem_->get_upper_bound(i));
        // Initialize the position and velocity vectors
        position_[i] = distr(*random_generator_);
        velocity_[i] = distr(*random_generator_);
    }
    // Initialize the best position
    best_position_ = position_;
    // Initialize the best value
    best_value_ = problem_->get_fitness_function()(position_);
    // Initialize the constraint violation
    update_constraint_violation();
    // Initialize the best constraint violation
    best_constraint_violation_ = constraint_violation_;
}

template <std::size_t dim>
void Particle<dim>::update(const RealVector<dim> &global_best_position, double violation_threshold, int iteration, int max_iter, double tol)
{
    // Compute beta avoiding division by zero
    double beta = (global_best_position - best_position_).norm();
    if (beta < 1e-8)
        beta = 1e-8;
    // Compute omega according to the current iteration
    double delta = (omega_s_ - omega_f_) / max_iter;
    double omega = (omega_s_ - omega_f_) * exp(-delta * iteration / beta) + omega_f_;
    // Compute phi1 and phi2 according to the current iteration
    delta = (phi1_s_ - phi1_f_) / max_iter;
    double phi1 = (phi1_s_ - phi1_f_) * exp(-delta * iteration / beta) + phi1_f_;
    // Compute phi2 according to the current iteration
    delta = (phi2_s_ - phi2_f_) / max_iter;
    double phi2 = (phi2_s_ - phi2_f_) * exp(delta * iteration / beta) + phi2_f_;

    // Compute the isobarycenter of the particle
    RealVector<dim> G = position_;
    G += (phi1 * (best_position_ - position_) + phi2 * (global_best_position - position_)) / 3.0;

    // Generate a point in the hypersphere of radius ||G-position|| centered in G
    double radius = (G - position_).norm();
    std::uniform_real_distribution<double> distr(0, 1);
    //  1. Generate a random direction in a unitary hypersphere
    RealVector<dim> random_direction = RealVector<dim>::NullaryExpr([&](int)
                                                                    { return distr(*random_generator_); });
    random_direction.normalize();
    //  2. Generate a random radius with probability proportional to the surface area of a ball with a given radius
    double random_radius = pow(distr(*random_generator_), 1.0 / dim);
    //  3. Generate the point in the affine hypersphere
    RealVector<dim> random_point = G + radius * (random_direction * random_radius);

    // Update the velocity
    velocity_ = omega * velocity_ + random_point - position_;
    // Update the position
    position_ = velocity_ + position_;

    // Modify each dimension of the position vector with its saturation energy
    for (size_t i = 0; i < dim; i++)
    {
        if (position_[i] < problem_->get_lower_bound(i))
            position_[i] = problem_->get_lower_bound(i);
        else if (position_[i] > problem_->get_upper_bound(i))
            position_[i] = problem_->get_upper_bound(i);
    }

    // Update the total constraint violation in the actual position
    update_constraint_violation();

    // Update the personal best position, value, and violation following the feasibility-based rule
    double current_value = problem_->get_fitness_function()(position_);
    if (feasibility_rule(current_value, best_value_, constraint_violation_, best_constraint_violation_, violation_threshold, tol))
    {
        best_position_ = position_;
        best_value_ = current_value;
        best_constraint_violation_ = constraint_violation_;
    }
}

template <std::size_t dim>
void Particle<dim>::print(std::ostream &out) const
{
    out << "Position: (";
    for (std::size_t i = 0; i < dim; ++i)
        out << position_[i] << ", ";
    out << "\b\b)" << std::endl;

    out << "Velocity: (";
    for (std::size_t i = 0; i < dim; ++i)
        out << velocity_[i] << ", ";
    out << "\b\b)" << std::endl;

    out << "Constraint violation:\t" << constraint_violation_ << std::endl;

    out << "Best position: (";
    for (std::size_t i = 0; i < dim; ++i)
        out << best_position_[i] << ", ";
    out << "\b\b)" << std::endl;

    out << "Best value:\t" << best_value_ << std::endl;

    out << "Best constraint violation:\t" << best_constraint_violation_ << std::endl;
}

template <size_t dim>
bool Particle<dim>::is_better_than(const Particle<dim> &other, double violation_threshold, double tol) const
{
    return feasibility_rule(best_value_, other.get_best_value(), best_constraint_violation_,
                            other.get_best_constraint_violation(), violation_threshold, tol);
}

template <size_t dim>
void Particle<dim>::update_constraint_violation()
{
    constraint_violation_ = 0.0;
    if (problem_->has_constraints())
    {
        for (RealFunction<dim> constraint : problem_->get_equality_constraints())
            constraint_violation_ += std::abs(constraint(position_));
        for (RealFunction<dim> constraint : problem_->get_inequality_constraints())
            constraint_violation_ += std::max(0.0, constraint(position_));
    }
}

template <size_t dim>
bool Particle<dim>::feasibility_rule(double value1, double value2, double viol1, double viol2, double violation_threshold, double tol) const
{
    double ub = violation_threshold + tol;
    double lb = std::max(violation_threshold - tol, 0.0);

    // check if one is feasible and if the other is not
    if (viol1 <= lb && viol2 > ub)
        return true;
    else if (viol1 > ub && viol2 <= lb)
        return false;
    // check if both are feasible, then the better one has the smaller value
    else if (viol1 <= lb && viol2 <= lb)
        return value1 < value2;
    // if both are infeasible, better has the smaller total violation
    else
        return viol1 < (viol2 - tol);
}