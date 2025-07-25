/**
 * @file
 * @brief Misc function related to player transformations.
**/

#pragma once

#include <set>

#include "enum.h"
#include "player.h"

constexpr int DRAGON_CLAWS = 3;
constexpr int DRAGON_FANGS = 5;

#define FLUX_ENERGY_KEY "flux_energy"
constexpr int FLUX_ENERGY_WARNING = 10;

enum form_capability
{
    FC_DEFAULT,
    FC_ENABLE,
    FC_FORBID
};

class FormAttackVerbs
{
public:
    FormAttackVerbs(const char *_weak, const char *_medium,
                    const char *_strong, const char *_devastating)
        : weak(_weak), medium(_medium), strong(_strong),
          devastating(_devastating)
    { }

    FormAttackVerbs(const char *msg)
        : weak(msg), medium(msg), strong(msg), devastating(msg)
    { }

public:
    const char * const weak;
    const char * const medium;
    const char * const strong;
    const char * const devastating;
};

class FormScaling {
public:
    FormScaling() : base(0), scaling(0), xl_based(false) {}
    FormScaling& Base(int b) { base = b; return *this; }
    FormScaling& Scaling(int s) { scaling = s; return *this; }
    FormScaling& XLBased() { xl_based = true; return *this; }

    int base;      // value at 0 XL/skill (as applicable)
    int scaling;   // value added to base at max XL/skill
    bool xl_based; // if false, scale on Shapeshifting skill
};

struct form_entry; // defined in form-data.h (private)
class Form
{
private:
    Form() = delete;
    DISALLOW_COPY_AND_ASSIGN(Form);
    Form(const form_entry &fe);
protected:
    Form(transformation tran);
public:
    bool slot_is_blocked(equipment_slot slot) const;
    bool can_wield() const { return !slot_is_blocked(SLOT_WEAPON); }

    int get_level(int scale) const;

    int mult_hp(int base_hp, bool force_talisman = false, int skill = -1) const;

    /// Is the player below the minimum skill for this form?
    bool underskilled() const { return get_level(1) < min_skill; }

    /**
     * What monster corresponds to this form?
     *
     * Used for console player glyphs.
     *
     * @return The monster_type corresponding to this form.
     */
    virtual monster_type get_equivalent_mons() const { return equivalent_mons; }

    /**
     * A name for the form longer than that used by the status light.
     *
     * E.g. "foo-form", "blade tentacles", &c. Used for dumps, morgues &
     * the % screen.
     *
     * @return The 'long name' of the form.
     */
    virtual string get_long_name() const { return long_name; }

    /**
     * A description of this form.
     *
     * E.g. "a fearsome dragon!", punctuation included.
     *
     * Used for the @ screen and, by default, transformation messages.
     *
     * @return A description of the form.
     */
    virtual string get_transform_description() const { return description; }

    virtual string get_description(bool past_tense = false) const;
    virtual string transform_message() const;
    virtual string get_untransform_message() const;

    virtual int res_fire() const;
    virtual int res_cold() const;
    int res_neg() const;
    bool res_elec() const;
    int res_pois() const;
    bool res_rot() const;
    bool res_corr() const;
    bool res_miasma() const;
    bool res_petrify() const;

    /**
     * Base unarmed damage provided by the form.
     */
    int get_base_unarmed_damage(bool random = true, int skill = -1) const;

    /// Damage done by a custom aux attack of this form.
    virtual int get_aux_damage(bool /*random*/ = true,
                               int /*skill*/ = -1) const {
        return 0;
    }

    /// Does this form care about skill for UC damage and accuracy, or only XL?
    virtual bool get_unarmed_uses_skill() const { return !unarmed_bonus_dam.xl_based; }

    /**
     * The brand of this form's unarmed attacks (SPWPN_FREEZING, etc).
     */
    virtual brand_type get_uc_brand() const { return uc_brand; }

    virtual string get_uc_attack_name(string default_name) const;
    virtual int slay_bonus(bool /*random*/ = true, int /*skill*/ = -1) const { return 0; }
    virtual int will_bonus() const { return 0; };
    virtual int get_ac_bonus(int skill = -1) const;
    virtual int ev_bonus(int /*skill*/ = -1) const;
    virtual int get_body_ac_mult(int /*skill*/ = -1) const;
    virtual int get_vamp_chance(int /*skill*/ = -1) const { return 0; }
    virtual int get_web_chance(int /*skill*/ = -1) const {return 0; }
    virtual int regen_bonus(int /*skill*/ = -1) const {return 0; }
    virtual int mp_regen_bonus(int /*skill*/ = -1) const {return 0; }
    virtual int max_mp_bonus(int /*skill*/ = -1) const {return 0; }
    virtual dice_def get_special_damage(bool /*random*/ = true, int /*skill*/ = -1) const;

    // XXX: Werewolf-specific
    virtual int get_werefury_kill_bonus(int /*skill*/ = -1) const { return 0; }
    virtual int get_takedown_multiplier(int /*skill*/ = -1) const { return 0; }
    virtual int get_howl_power(int /*skill*/ = -1) const { return 0; }

    virtual int get_effect_size(int /*skill*/ = -1) const { return 0; }
    virtual int get_effect_chance(int /*skill*/ = -1) const { return 0; }

    bool enables_flight() const;
    bool forbids_flight() const;

    bool player_can_fly() const;
    bool player_can_swim() const;
    bool player_likes_water() const;
    bool can_offhand_punch() const;

    string player_prayer_action() const;
    string melding_description(bool itemized) const;

    virtual vector<pair<string, string>> get_fakemuts() const;
    virtual vector<pair<string, string>> get_bad_fakemuts() const;

public:
    /// Status light ("Foo"); "" for none
    const string short_name;
    /// "foo"; used for wizmode transformation dialogue
    const string wiz_name;

    /// The skill level below which the player gets HP penalties for using the form.
    const int min_skill;
    /// The skill level beyond which further skill provides no benefit.
    const int max_skill;

    /// flat str bonus
    const int str_mod;
    /// flat dex bonus
    const int dex_mod;

    /// base move speed
    const int base_move_speed;

    /// Equipment types unusable in this form.
    /** A bitfield representing a union of (1 << equipment_slot) values for
     * equipment slots that are melded in this form.
     */
    const int blocked_slots; // XX check enum size at compile time?
    /// size of the form
    const size_type size;

    /// can the player cast while in this form?
    const bool can_cast;

    /// colour of 'weapon' in UI
    const int uc_colour;
    /// a set of verbs to use based on damage done, when using UC in this form
    const FormAttackVerbs uc_attack_verbs;

    /// "Used to mark forms which keep most form-based mutations."
    const bool changes_anatomy;
    //
    const bool changes_substance;
    /// What does this form change the player's holiness to? (MH_NONE if it leaves it unchanged)
    /// Note that vampire is special-cased in player::holiness().
    const mon_holy_type holiness;
    /// Is this form considered 'bad' (ie: hostile)
    const bool is_badform;

    /// Does this form have blood (used for sublimation and bloodsplatters)?
    const form_capability has_blood;
    /// Does this form have hair?
    const form_capability has_hair;
    /// Does this form have bones?
    const form_capability has_bones;
    /// Does this form have feet?
    const form_capability has_feet;
    /// Does this form have ears?
    const form_capability has_ears;

    /// what verb does the player use when shouting in this form?
    const string shout_verb;
    /// a flat bonus (or penalty) to shout volume
    const int shout_volume_modifier;

    /// The name of this form's hand-equivalents; "" defaults to species.
    const string hand_name;
    /// The name of this form's foot-equivalents; "" defaults to species.
    const string foot_name;
    /// The name of this form's flesh-equivalent; "" defaults to species.
    const string flesh_equivalent;

    /// Identifier for the meaning of the special dice for this form (for the
    /// form properties screen).
    const string special_dice_name;

protected:
    /// See Form::get_long_name().
    const string long_name;
    /// See Form::get_transform_description().
    const string description;

    /// Resistances granted by this form.
    /** A bitfield holding a union of mon_resist_flags for resists granted
     * by the form.
     */
    const int resists;

    /// bonuses to AC when in this form, potentially scaling with skill or XL
    const FormScaling ac;

    /// bonuses to EV when in this form, potentially scaling with skill or XL
    const FormScaling ev;

    /// mulplier to base body armour AC when in this form, potentially scaling with skill or XL
    const FormScaling body_ac_mult;

    /// See Form::get_base_unarmed_damage().
    const FormScaling unarmed_bonus_dam;

    vector<pair<string,string>> fakemuts;
    vector<pair<string,string>> badmuts;

    /// Calculate the given FormScaling for this form, multiplied by scale.
    int scaling_value(const FormScaling &sc, bool random,
                      int level = -1, int scale = 1) const;
    /// Calculate the given FormScaling for this form, with math internally multiplied by scale.
    int divided_scaling(const FormScaling &sc, bool random,
                        int level = -1, int scale = 1) const;

private:
    /// Can this form fly?
    /** Whether the form enables, forbids, or does nothing to the player's
     * ability to fly.
     */
    const form_capability can_fly;
    /// Can this form swim?
    /** Whether the form enables, forbids, or does nothing to the player's
     * ability to swim (traverse deep water).
     */
    const form_capability can_swim;

    const form_capability offhand_punch;

    /// See Form::get_uc_brand().
    const brand_type uc_brand;
    /// the name of the uc 'weapon' in the HUD; "" uses species defaults.
    const string uc_attack;

    /// Altar prayer action; "" uses defaults. See Form::player_prayer_action()
    const string prayer_action;

    /// See Form::get_equivalent_mons().
    const monster_type equivalent_mons;

    /// 100 * multiplier to hp/mhp (that is, 100 is base, 150 is 1.5x, etc)
    const int hp_mod;

    /// Calculator for form-specific 'special' damage done by this form (eg:
    /// Blinkbolt damage or Contam damage)
    const dam_deducer* special_dice;
};
const Form* get_form(transformation form = you.form);
const Form* cur_form(bool temp = true);

bool lifeless_prevents_form(transformation form = you.form);

bool form_is_bad(transformation form = you.form);
bool form_can_wield(transformation form = you.form);
bool form_can_wear(transformation form = you.form);
bool form_can_fly(transformation form = you.form);
bool form_can_swim(transformation form = you.form);
bool form_changes_substance(transformation form = you.form);
bool form_changes_anatomy(transformation form = you.form);
bool form_has_blood(transformation form = you.form);
bool form_has_hair(transformation form = you.form);
bool form_has_bones(transformation form = you.form);
bool form_has_feet(transformation form = you.form);
bool form_has_ears(transformation form = you.form);

bool feat_dangerous_for_form(transformation which_trans,
                             dungeon_feature_type feat,
                             const item_def* talisman = nullptr);
bool transforming_is_unsafe(transformation which_trans);

string cant_transform_reason(transformation which_trans, bool involuntary = false,
                             bool temp = true);
bool check_transform_into(transformation which_trans, bool involuntary = false,
                          const item_def* talisman = nullptr);
bool transform(int dur, transformation which_trans, bool involuntary = false,
               bool using_talisman = false);

// skip_move: don't make player re-enter current cell
void untransform(bool skip_move = false, bool scale_hp = true,
                 bool preserve_equipment = false,
                 transformation new_form = transformation::none);

void unset_default_form();
void set_default_form(transformation t, const item_def *source);

void set_form(transformation which_trans, int dur, bool scale_hp = true);
void return_to_default_form(bool new_form = false);

monster_type transform_mons();
string blade_parts(bool terse = false);
const char* transform_name(transformation form = you.form);

void merfolk_check_swimming(dungeon_feature_type old_grid,
                            bool stepped = false);
void merfolk_start_swimming(bool step = false);
void merfolk_stop_swimming();

transformation form_for_talisman(const item_def &talisman);
void clear_form_info_on_exit();

void sphinx_notice_riddle_target(monster* mon);
void sphinx_check_riddle();

void sun_scarab_spawn_ember(bool first_time);
monster* get_solar_ember();

bool maw_growl_check(const monster* mon);
