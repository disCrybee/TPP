#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// -------------------- Mandatory task: Library --------------------
class Book {
public:
    Book(std::string title, std::string author)
        : title_(std::move(title)), author_(std::move(author)) {}

    const std::string& title() const noexcept { return title_; }
    const std::string& author() const noexcept { return author_; }

    void display(std::ostream& out) const {
        out << "Title: " << title_ << ", Author: " << author_ << '\n';
    }

private:
    std::string title_;
    std::string author_;
};

class IBookRepository {
public:
    virtual ~IBookRepository() = default;
    virtual void save(const Book& book) = 0;
};

class IBookNotifier {
public:
    virtual ~IBookNotifier() = default;
    virtual void notifyNewBook(const Book& book) = 0;
};

class InMemoryBookRepository : public IBookRepository {
public:
    void save(const Book& book) override { saved_.push_back(book); }
    const std::vector<Book>& saved() const noexcept { return saved_; }
private:
    std::vector<Book> saved_;
};

class ConsoleBookNotifier : public IBookNotifier {
public:
    void notifyNewBook(const Book& book) override {
        std::cout << "Notification: new book \"" << book.title() << "\"\n";
    }
};

class Library {
public:
    Library(IBookRepository& repository, IBookNotifier& notifier)
        : repository_(repository), notifier_(notifier) {}

    void addBook(const Book& book) {
        books_.push_back(book);
        repository_.save(book);
        notifier_.notifyNewBook(book);
    }

    void displayBooks(std::ostream& out) const {
        for (const auto& book : books_) book.display(out);
    }
    std::size_t size() const noexcept { return books_.size(); }

private:
    std::vector<Book> books_;
    IBookRepository& repository_;
    IBookNotifier& notifier_;
};

// -------------------- Kata 2: Gilded Rose --------------------
struct Item {
    std::string name;
    int sellIn;
    int quality;
};

class IQualityRule {
public:
    virtual ~IQualityRule() = default;
    virtual void update(Item& item) const = 0;
};

namespace {
void increase(Item& item, int amount = 1) {
    item.quality = std::min(50, item.quality + amount);
}
void decrease(Item& item, int amount = 1) {
    item.quality = std::max(0, item.quality - amount);
}
class NormalRule final : public IQualityRule {
public:
    void update(Item& item) const override {
        decrease(item, item.sellIn <= 0 ? 2 : 1);
        --item.sellIn;
    }
};
class AgedBrieRule final : public IQualityRule {
public:
    void update(Item& item) const override {
        increase(item, item.sellIn <= 0 ? 2 : 1);
        --item.sellIn;
    }
};
class SulfurasRule final : public IQualityRule {
public:
    void update(Item& item) const override { item.quality = 80; }
};
class BackstagePassRule final : public IQualityRule {
public:
    void update(Item& item) const override {
        if (item.sellIn <= 0) item.quality = 0;
        else {
            int gain = item.sellIn <= 5 ? 3 : (item.sellIn <= 10 ? 2 : 1);
            increase(item, gain);
        }
        --item.sellIn;
    }
};
// Individual extension: Conjured items degrade twice as fast as ordinary items.
class ConjuredRule final : public IQualityRule {
public:
    void update(Item& item) const override {
        decrease(item, item.sellIn <= 0 ? 4 : 2);
        --item.sellIn;
    }
};
} // namespace

class QualityUpdater {
public:
    QualityUpdater() {
        rules_.emplace_back("Aged Brie", std::make_unique<AgedBrieRule>());
        rules_.emplace_back("Sulfuras, Hand of Ragnaros", std::make_unique<SulfurasRule>());
        rules_.emplace_back("Backstage passes to a TAFKAL80ETC concert",
                            std::make_unique<BackstagePassRule>());
        rules_.emplace_back("Conjured Mana Cake", std::make_unique<ConjuredRule>());
    }
    void update(Item& item) const {
        for (const auto& entry : rules_) {
            if (entry.first == item.name) {
                entry.second->update(item);
                return;
            }
        }
        normal_.update(item);
    }
private:
    std::vector<std::pair<std::string, std::unique_ptr<IQualityRule>>> rules_;
    NormalRule normal_;
};

class GildedRose {
public:
    explicit GildedRose(std::vector<Item> items) : items_(std::move(items)) {}
    void updateQuality() {
        for (auto& item : items_) updater_.update(item);
    }
    const std::vector<Item>& items() const noexcept { return items_; }
private:
    std::vector<Item> items_;
    QualityUpdater updater_;
};

void testLibrary() {
    InMemoryBookRepository repository;
    class TestNotifier : public IBookNotifier {
    public:
        void notifyNewBook(const Book&) override { ++calls; }
        int calls = 0;
    } notifier;
    Library library(repository, notifier);
    library.addBook(Book("The Catcher in the Rye", "J.D. Salinger"));
    library.addBook(Book("To Kill a Mockingbird", "Harper Lee"));
    assert(library.size() == 2);
    assert(repository.saved().size() == 2);
    assert(notifier.calls == 2);
}

void testGildedRose() {
    QualityUpdater updater;
    Item normal{"Elixir", 5, 10};
    updater.update(normal);
    assert(normal.sellIn == 4 && normal.quality == 9);
    Item expired{"Elixir", 0, 10};
    updater.update(expired);
    assert(expired.sellIn == -1 && expired.quality == 8);
    Item brie{"Aged Brie", 2, 48};
    updater.update(brie);
    assert(brie.sellIn == 1 && brie.quality == 49);
    Item brieExpired{"Aged Brie", 0, 48};
    updater.update(brieExpired);
    assert(brieExpired.quality == 50);
    Item sulfuras{"Sulfuras, Hand of Ragnaros", 0, 80};
    updater.update(sulfuras);
    assert(sulfuras.sellIn == 0 && sulfuras.quality == 80);
    Item backstage10{"Backstage passes to a TAFKAL80ETC concert", 10, 20};
    updater.update(backstage10);
    assert(backstage10.quality == 22);
    Item backstage5{"Backstage passes to a TAFKAL80ETC concert", 5, 20};
    updater.update(backstage5);
    assert(backstage5.quality == 23);
    Item backstageExpired{"Backstage passes to a TAFKAL80ETC concert", 0, 20};
    updater.update(backstageExpired);
    assert(backstageExpired.quality == 0);
    Item conjured{"Conjured Mana Cake", 3, 10};
    updater.update(conjured);
    assert(conjured.sellIn == 2 && conjured.quality == 8);
    Item conjuredExpired{"Conjured Mana Cake", 0, 10};
    updater.update(conjuredExpired);
    assert(conjuredExpired.quality == 6);
    Item zero{"Elixir", 3, 0};
    updater.update(zero);
    assert(zero.quality == 0);
    Item capped{"Aged Brie", 3, 50};
    updater.update(capped);
    assert(capped.quality == 50);
}

int main() {
    testLibrary();
    testGildedRose();
    std::cout << "All tests passed.\n\n";
    InMemoryBookRepository repository;
    ConsoleBookNotifier notifier;
    Library library(repository, notifier);
    library.addBook(Book("The Catcher in the Rye", "J.D. Salinger"));
    library.addBook(Book("To Kill a Mockingbird", "Harper Lee"));
    std::cout << "Library catalogue:\n";
    library.displayBooks(std::cout);
    std::vector<Item> sample{
        {"Elixir", 5, 10},
        {"Aged Brie", 2, 0},
        {"Conjured Mana Cake", 3, 10}
    };
    GildedRose shop(std::move(sample));
    shop.updateQuality();
    std::cout << "\nItems after one day:\n";
    for (const auto& item : shop.items())
        std::cout << item.name << " | SellIn=" << item.sellIn
                  << " | Quality=" << item.quality << '\n';
    return 0;
}
